// Kirk
// Copyright (C) 2023  Pavel Sobolev <paveloom@riseup.net>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/kirk-qobuz-client.h"

#include "include/config.h"
#include "src/kirk-preferences-window.h"
#include "src/kirk-secret-schema.h"
#include "src/kirk-uri.h"

#include <libsoup/soup.h>

typedef enum {
    KIRK_QOBUZ_CLIENT_STATUS_CANCELLED,
    KIRK_QOBUZ_CLIENT_STATUS_INVALID_MESSAGE,
    KIRK_QOBUZ_CLIENT_STATUS_FAILED_CONNECTION,

    KIRK_QOBUZ_CLIENT_STATUS_MISSING_TOKEN,
    KIRK_QOBUZ_CLIENT_STATUS_MISSING_APP_ID,
    KIRK_QOBUZ_CLIENT_STATUS_FAILED_AUTHORIZATION,
    KIRK_QOBUZ_CLIENT_STATUS_SUCCESSFUL_AUTHORIZATION,

    KIRK_QOBUZ_CLIENT_STATUS_FAILED_TO_FIND_THE_BUNDLE_URL,
    KIRK_QOBUZ_CLIENT_STATUS_FAILED_TO_FIND_THE_APP_ID,
    KIRK_QOBUZ_CLIENT_STATUS_FETCHED_THE_APP_ID,
} KirkQobuzClientStatus;

typedef struct {
    SoupSession* session;

    gchar* app_id;
    gchar* token;

    KirkQobuzClientStatus status;
} KirkQobuzClient;

static void kirk_qobuz_client_free(KirkQobuzClient* self) {
    g_object_unref(self->session);
    secret_password_free(self->token);
    secret_password_free(self->app_id);
    g_free(self);
}

static void kirk_qobuz_client_return_result(GTask* task) {
    const KirkQobuzClient* const self = g_task_get_task_data(task);

    gchar* message = NULL;
    switch (self->status) {
    case KIRK_QOBUZ_CLIENT_STATUS_CANCELLED:
        message = "Qobuz: operation was cancelled!";
        break;
    case KIRK_QOBUZ_CLIENT_STATUS_INVALID_MESSAGE:
        message = "Qobuz: invalid message!";
        break;
    case KIRK_QOBUZ_CLIENT_STATUS_FAILED_CONNECTION:
        message = "Qobuz: couldn't connect to the server!";
        break;

    case KIRK_QOBUZ_CLIENT_STATUS_MISSING_TOKEN:
        message = "Qobuz: coudln't lookup the token!";
        break;
    case KIRK_QOBUZ_CLIENT_STATUS_MISSING_APP_ID:
        message = "Qobuz: coudln't lookup the app ID!";
        break;
    case KIRK_QOBUZ_CLIENT_STATUS_FAILED_AUTHORIZATION:
        message = "Qobuz: authorization failed!";
        break;
    case KIRK_QOBUZ_CLIENT_STATUS_SUCCESSFUL_AUTHORIZATION:
        message = "Qobuz: successful authorization!";
        break;

    case KIRK_QOBUZ_CLIENT_STATUS_FAILED_TO_FIND_THE_BUNDLE_URL:
        message = "Qobuz: failed to find the bundle URL!";
        break;
    case KIRK_QOBUZ_CLIENT_STATUS_FAILED_TO_FIND_THE_APP_ID:
        message = "Qobuz: failed to find the application ID!";
        break;
    case KIRK_QOBUZ_CLIENT_STATUS_FETCHED_THE_APP_ID:
        message = "Qobuz: fetched the application ID!";
        break;

    default:
        message = "Qobuz: unknown error!";
        break;
    }

    g_task_return_pointer(task, message, NULL);
    g_object_unref(task);
}

#define kirk_qobuz_client_return_if_window_closed(task)                        \
    if (!gtk_widget_is_visible(GTK_WIDGET(g_task_get_source_object(task)))) {  \
        g_task_return_pointer(task, NULL, NULL);                               \
        g_object_unref(task);                                                  \
        return;                                                                \
    }

#define kirk_qobuz_client_return_if_cancelled(self, task)                      \
    if (g_cancellable_is_cancelled(g_task_get_cancellable(task))) {            \
        (self)->status = KIRK_QOBUZ_CLIENT_STATUS_CANCELLED;                   \
        kirk_qobuz_client_return_result(task);                                 \
        return;                                                                \
    }

#define QOBUZ_SCHEME "https://"
#define QOBUZ_MAIN_HOST "www.qobuz.com"
#define QOBUZ_PLAY_HOST "play.qobuz.com"

#define QOBUZ_API_PATH "/api.json/0.2"
#define QOBUZ_LOGIN_PATH QOBUZ_API_PATH "/user/login"

// Authorization sequence

static void kirk_qobuz_client_send_authorization_request_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* const self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    soup_session_send_finish(self->session, result, NULL);
    SoupMessage* msg =
        soup_session_get_async_result_message(self->session, result);

    if (!msg) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_FAILED_CONNECTION;
        kirk_qobuz_client_return_result(task);
        return;
    }

    const SoupStatus status = soup_message_get_status(msg);
    if (status == SOUP_STATUS_OK) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_SUCCESSFUL_AUTHORIZATION;
    } else {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_FAILED_AUTHORIZATION;
    }

    kirk_qobuz_client_return_result(task);
}

static void kirk_qobuz_client_send_authorization_request(GTask* task) {
    KirkQobuzClient* const self = g_task_get_task_data(task);

    const gchar* const url = QOBUZ_SCHEME QOBUZ_MAIN_HOST QOBUZ_LOGIN_PATH;
    SoupMessage* const msg = soup_message_new(SOUP_METHOD_GET, url);

    if (!msg) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_INVALID_MESSAGE;
        kirk_qobuz_client_return_result(task);
        return;
    }

    SoupMessageHeaders* request_headers = soup_message_get_request_headers(msg);
    const g_autofree gchar* authorization_header_value =
        g_strconcat("Bearer ", self->token, NULL);
    soup_message_headers_append(
        request_headers,
        "Authorization",
        authorization_header_value
    );
    soup_message_headers_append(request_headers, "X-App-ID", self->app_id);

    soup_session_send_async(
        self->session,
        msg,
        G_PRIORITY_DEFAULT,
        g_task_get_cancellable(task),
        kirk_qobuz_client_send_authorization_request_finish,
        task
    );
}

static void kirk_qobuz_client_lookup_app_id_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* const self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    gchar* app_id = kirk_secret_schema_lookup_password_finish(result, NULL);

    if (!app_id || !app_id[0]) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_MISSING_TOKEN;
        kirk_qobuz_client_return_result(task);
        return;
    }

    self->app_id = app_id;

    kirk_qobuz_client_send_authorization_request(task);
}

static void kirk_qobuz_client_lookup_app_id(GTask* task) {
    kirk_secret_schema_lookup_password(
        "qobuz",
        "app_id",
        g_task_get_cancellable(task),
        kirk_qobuz_client_lookup_app_id_finish,
        task
    );
}

static void kirk_qobuz_client_lookup_token_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* const self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    gchar* token = kirk_secret_schema_lookup_password_finish(result, NULL);

    if (!token || !token[0]) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_MISSING_TOKEN;
        kirk_qobuz_client_return_result(task);
        return;
    }

    self->token = token;

    kirk_qobuz_client_lookup_app_id(task);
}

static void kirk_qobuz_client_lookup_token(GTask* task) {
    kirk_secret_schema_lookup_password(
        "qobuz",
        "token",
        g_task_get_cancellable(task),
        kirk_qobuz_client_lookup_token_finish,
        task
    );
}

void kirk_qobuz_client_try_to_authorize(
    GObject* source_object,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    KirkQobuzClient* qobuz_client = g_malloc0(sizeof(KirkQobuzClient));

    qobuz_client->session = soup_session_new();

    GTask* task = g_task_new(source_object, cancellable, callback, user_data);
    g_task_set_check_cancellable(task, FALSE);
    g_task_set_task_data(
        task,
        qobuz_client,
        (GDestroyNotify)kirk_qobuz_client_free
    );

    kirk_qobuz_client_lookup_token(task);
}

// Fetch application ID sequence

typedef struct {
    gchar* (*regex_search_function)(GBytes*);
    GBytes* bytes;
} RegexSearchClosure;

static void regex_search_data_free(RegexSearchClosure* closure) {
    g_bytes_unref(closure->bytes);
    g_free(closure);
}

static void regex_search_thread_cb(
    GTask* task,
    gpointer source_object,
    gpointer task_data,
    GCancellable* cancellable
) {
    RegexSearchClosure* closure = task_data;

    gchar* match = closure->regex_search_function(closure->bytes);
    g_task_return_pointer(task, match, NULL);
}

static void regex_search_async(
    RegexSearchClosure* closure,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    GTask* task = g_task_new(NULL, cancellable, callback, user_data);
    g_task_set_return_on_cancel(task, FALSE);
    g_task_set_task_data(task, closure, (GDestroyNotify)regex_search_data_free);

    g_task_run_in_thread(task, regex_search_thread_cb);

    g_object_unref(task);
}

static void kirk_qobuz_client_find_the_app_id_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* const self = g_task_get_task_data(task);

    const g_autofree gchar* app_id =
        g_task_propagate_pointer(G_TASK(result), NULL);

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    if (app_id) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_FETCHED_THE_APP_ID;
    } else {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_FAILED_TO_FIND_THE_APP_ID;
        kirk_qobuz_client_return_result(task);
        return;
    }

    kirk_secret_schema_store_qobuz_app_id(app_id);

    kirk_qobuz_client_return_result(task);
}

static gchar* kirk_qobuz_client_find_the_app_id(GBytes* bytes) {
    const gchar* body = g_bytes_get_data(bytes, NULL);

    g_autoptr(GRegex) regex = g_regex_new(
        "production:\\{api:\\{appId:\"(\\d{9})\",appSecret:\"\\w{32}",
        G_REGEX_DEFAULT,
        G_REGEX_MATCH_DEFAULT,
        NULL
    );

    g_autoptr(GMatchInfo) match_info = NULL;
    const gboolean matched = g_regex_match(regex, body, 0, &match_info);

    if (!matched) {
        return NULL;
    }

    return g_match_info_fetch(match_info, 1);
}

static void kirk_qobuz_client_find_the_app_id_async(
    GBytes* bytes,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    RegexSearchClosure* data = g_malloc(sizeof(RegexSearchClosure));
    data->regex_search_function = kirk_qobuz_client_find_the_app_id;
    data->bytes = bytes;

    regex_search_async(data, cancellable, callback, user_data);
}

static void kirk_qobuz_client_fetch_the_bundle_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* const self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    GBytes* bytes =
        soup_session_send_and_read_finish(self->session, result, NULL);

    if (!bytes) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_FAILED_CONNECTION;
        kirk_qobuz_client_return_result(task);
        return;
    }

    kirk_qobuz_client_find_the_app_id_async(
        bytes,
        g_task_get_cancellable(task),
        kirk_qobuz_client_find_the_app_id_finish,
        task
    );
}

static void kirk_qobuz_client_fetch_the_bundle(
    GTask* task,
    const gchar* bundle_url
) {
    KirkQobuzClient* const self = g_task_get_task_data(task);

    SoupMessage* const msg = soup_message_new(SOUP_METHOD_GET, bundle_url);

    if (!msg) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_INVALID_MESSAGE;
        kirk_qobuz_client_return_result(task);
        return;
    }

    soup_session_send_and_read_async(
        self->session,
        msg,
        G_PRIORITY_DEFAULT,
        g_task_get_cancellable(task),
        kirk_qobuz_client_fetch_the_bundle_finish,
        task
    );
}

static void kirk_qobuz_client_find_the_bundle_url(GTask* task, GBytes* bytes) {
    KirkQobuzClient* const self = g_task_get_task_data(task);

    const gchar* body = g_bytes_get_data(bytes, NULL);

    g_autoptr(GRegex) regex = g_regex_new(
        "<script src=\"(/resources/\\d+\\.\\d+\\.\\d+-[a-z]\\d{3}/bundle\\.js)\"></script>",
        G_REGEX_DEFAULT,
        G_REGEX_MATCH_DEFAULT,
        NULL
    );

    g_autoptr(GMatchInfo) match_info = NULL;
    const gboolean matched = g_regex_match(regex, body, 0, &match_info);

    if (!matched) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_FAILED_TO_FIND_THE_BUNDLE_URL;
        kirk_qobuz_client_return_result(task);
        return;
    }

    const g_autofree gchar* bundle_path = g_match_info_fetch(match_info, 1);
    const g_autofree gchar* bundle_url =
        g_strconcat(QOBUZ_SCHEME QOBUZ_PLAY_HOST, bundle_path, NULL);

    kirk_qobuz_client_fetch_the_bundle(task, bundle_url);

    g_bytes_unref(bytes);
}

static void kirk_qobuz_client_fetch_the_play_host_page_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* const self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    GBytes* bytes =
        soup_session_send_and_read_finish(self->session, result, NULL);

    if (!bytes) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_FAILED_CONNECTION;
        kirk_qobuz_client_return_result(task);
        return;
    }

    kirk_qobuz_client_find_the_bundle_url(task, bytes);
}

static void kirk_qobuz_client_fetch_the_play_host_page(GTask* task) {
    KirkQobuzClient* const self = g_task_get_task_data(task);

    const gchar* const url = QOBUZ_SCHEME QOBUZ_PLAY_HOST;
    SoupMessage* const msg = soup_message_new(SOUP_METHOD_GET, url);

    if (!msg) {
        self->status = KIRK_QOBUZ_CLIENT_STATUS_INVALID_MESSAGE;
        kirk_qobuz_client_return_result(task);
        return;
    }

    soup_session_send_and_read_async(
        self->session,
        msg,
        G_PRIORITY_DEFAULT,
        g_task_get_cancellable(task),
        kirk_qobuz_client_fetch_the_play_host_page_finish,
        task
    );
}

void kirk_qobuz_client_try_to_fetch_app_id(
    GObject* source_object,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    KirkQobuzClient* qobuz_client = g_malloc0(sizeof(KirkQobuzClient));
    qobuz_client->session = soup_session_new();

    GTask* task = g_task_new(source_object, cancellable, callback, NULL);
    g_task_set_check_cancellable(task, FALSE);
    g_task_set_task_data(
        task,
        qobuz_client,
        (GDestroyNotify)kirk_qobuz_client_free
    );

    kirk_qobuz_client_fetch_the_play_host_page(task);
}
