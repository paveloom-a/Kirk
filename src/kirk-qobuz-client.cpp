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

enum class KirkQobuzClientStatus {
    cancelled,
    invalid_message,
    failed_connection,

    missing_token,
    missing_app_id,
    failed_authorization,
    successful_authorization,

    failed_to_find_the_bundle_url,
    failed_to_find_the_app_id,
    fetched_the_app_id,
};

struct KirkQobuzClient {
    SoupSession* session = {};

    gchar* app_id = {};
    gchar* token = {};

    KirkQobuzClientStatus status;
};

#define KIRK_QOBUZ_CLIENT(o) (static_cast<KirkQobuzClient*>(o))

static void kirk_qobuz_client_free(KirkQobuzClient* self) {
    g_object_unref(self->session);
    secret_password_free(self->token);
    secret_password_free(self->app_id);
    delete self;
}

static void kirk_qobuz_client_return_result(GTask* task) {
    const KirkQobuzClient* const self =
        KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    const gchar* message = nullptr;
    switch (self->status) {
    case KirkQobuzClientStatus::cancelled:
        message = "Qobuz: operation was cancelled!";
        break;
    case KirkQobuzClientStatus::invalid_message:
        message = "Qobuz: invalid message!";
        break;
    case KirkQobuzClientStatus::failed_connection:
        message = "Qobuz: couldn't connect to the server!";
        break;

    case KirkQobuzClientStatus::missing_token:
        message = "Qobuz: coudln't lookup the token!";
        break;
    case KirkQobuzClientStatus::missing_app_id:
        message = "Qobuz: coudln't lookup the app ID!";
        break;
    case KirkQobuzClientStatus::failed_authorization:
        message = "Qobuz: authorization failed!";
        break;
    case KirkQobuzClientStatus::successful_authorization:
        message = "Qobuz: successful authorization!";
        break;

    case KirkQobuzClientStatus::failed_to_find_the_bundle_url:
        message = "Qobuz: failed to find the bundle URL!";
        break;
    case KirkQobuzClientStatus::failed_to_find_the_app_id:
        message = "Qobuz: failed to find the application ID!";
        break;
    case KirkQobuzClientStatus::fetched_the_app_id:
        message = "Qobuz: fetched the application ID!";
        break;

    default:
        message = "Qobuz: unknown error!";
        break;
    }

    g_task_return_pointer(task, (gpointer)(message), nullptr);
    g_object_unref(task);
}

#define kirk_qobuz_client_return_if_window_closed(task)                        \
    if (!gtk_widget_is_visible(GTK_WIDGET(g_task_get_source_object(task)))) {  \
        g_task_return_pointer(task, nullptr, nullptr);                         \
        g_object_unref(task);                                                  \
        return;                                                                \
    }

#define kirk_qobuz_client_return_if_cancelled(self, task)                      \
    if (g_cancellable_is_cancelled(g_task_get_cancellable(task))) {            \
        (self)->status = KirkQobuzClientStatus::cancelled;                     \
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
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    soup_session_send_finish(self->session, result, nullptr);
    SoupMessage* msg =
        soup_session_get_async_result_message(self->session, result);

    if (!msg) {
        self->status = KirkQobuzClientStatus::failed_connection;
        kirk_qobuz_client_return_result(task);
        return;
    }

    const SoupStatus status = soup_message_get_status(msg);
    if (status == SOUP_STATUS_OK) {
        self->status = KirkQobuzClientStatus::successful_authorization;
    } else {
        self->status = KirkQobuzClientStatus::failed_authorization;
    }

    kirk_qobuz_client_return_result(task);
}

static void kirk_qobuz_client_send_authorization_request(GTask* task) {
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    const gchar* const url = QOBUZ_SCHEME QOBUZ_MAIN_HOST QOBUZ_LOGIN_PATH;
    SoupMessage* const msg = soup_message_new(SOUP_METHOD_GET, url);

    if (!msg) {
        self->status = KirkQobuzClientStatus::invalid_message;
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
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    gchar* app_id = kirk_secret_schema_lookup_password_finish(result, nullptr);

    if (!app_id || !app_id[0]) {
        self->status = KirkQobuzClientStatus::missing_token;
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
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    gchar* token = kirk_secret_schema_lookup_password_finish(result, nullptr);

    if (!token || !token[0]) {
        self->status = KirkQobuzClientStatus::missing_token;
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
    auto qobuz_client = new KirkQobuzClient;

    qobuz_client->session = soup_session_new();

    GTask* task = g_task_new(source_object, cancellable, callback, user_data);
    g_task_set_check_cancellable(task, false);
    g_task_set_task_data(
        task,
        qobuz_client,
        reinterpret_cast<GDestroyNotify>(kirk_qobuz_client_free)
    );

    kirk_qobuz_client_lookup_token(task);
}

// Fetch application ID sequence

struct RegexSearchClosure {
    const gchar* (*regex_search_function)(GBytes*);
    GBytes* bytes;
};

static void regex_search_data_free(RegexSearchClosure* closure) {
    g_bytes_unref(closure->bytes);
    delete closure;
}

static void regex_search_thread_cb(
    GTask* task,
    gpointer source_object,
    gpointer task_data,
    GCancellable* cancellable
) {
    auto closure = static_cast<RegexSearchClosure*>(task_data);

    const gchar* match = closure->regex_search_function(closure->bytes);
    g_task_return_pointer(task, (gpointer)match, nullptr);
}

static void regex_search_async(
    RegexSearchClosure* closure,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    GTask* task = g_task_new(nullptr, cancellable, callback, user_data);
    g_task_set_return_on_cancel(task, false);
    g_task_set_task_data(
        task,
        closure,
        reinterpret_cast<GDestroyNotify>(regex_search_data_free)
    );

    g_task_run_in_thread(task, regex_search_thread_cb);

    g_object_unref(task);
}

static void kirk_qobuz_client_find_the_app_id_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    auto g_autofree app_id = static_cast<const gchar*>(  //
        g_task_propagate_pointer(G_TASK(result), nullptr)
    );

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    if (app_id) {
        self->status = KirkQobuzClientStatus::fetched_the_app_id;
    } else {
        self->status = KirkQobuzClientStatus::failed_to_find_the_app_id;
        kirk_qobuz_client_return_result(task);
        return;
    }

    kirk_secret_schema_store_qobuz_app_id(app_id);

    kirk_qobuz_client_return_result(task);
}

static const gchar* kirk_qobuz_client_find_the_app_id(GBytes* bytes) {
    auto body = static_cast<const gchar*>(g_bytes_get_data(bytes, nullptr));

    g_autoptr(GRegex) regex = g_regex_new(
        "production:\\{api:\\{appId:\"(\\d{9})\",appSecret:\"\\w{32}",
        G_REGEX_DEFAULT,
        G_REGEX_MATCH_DEFAULT,
        nullptr
    );

    g_autoptr(GMatchInfo) match_info = nullptr;
    const gboolean matched =
        g_regex_match(regex, body, G_REGEX_MATCH_DEFAULT, &match_info);

    if (!matched) {
        return nullptr;
    }

    return g_match_info_fetch(match_info, 1);
}

static void kirk_qobuz_client_find_the_app_id_async(
    GBytes* bytes,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    auto data =
        new RegexSearchClosure {kirk_qobuz_client_find_the_app_id, bytes};
    regex_search_async(data, cancellable, callback, user_data);
}

static void kirk_qobuz_client_fetch_the_bundle_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    GBytes* bytes =
        soup_session_send_and_read_finish(self->session, result, nullptr);

    if (!bytes) {
        self->status = KirkQobuzClientStatus::failed_connection;
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
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    SoupMessage* const msg = soup_message_new(SOUP_METHOD_GET, bundle_url);

    if (!msg) {
        self->status = KirkQobuzClientStatus::invalid_message;
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
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    auto body = static_cast<const gchar*>(g_bytes_get_data(bytes, nullptr));

    g_autoptr(GRegex) regex = g_regex_new(
        "<script src=\"(/resources/\\d+\\.\\d+\\.\\d+-[a-z]\\d{3}/bundle\\.js)\"></script>",
        G_REGEX_DEFAULT,
        G_REGEX_MATCH_DEFAULT,
        nullptr
    );

    g_autoptr(GMatchInfo) match_info = nullptr;
    const gboolean matched =
        g_regex_match(regex, body, G_REGEX_MATCH_DEFAULT, &match_info);

    if (!matched) {
        self->status = KirkQobuzClientStatus::failed_to_find_the_bundle_url;
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
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    kirk_qobuz_client_return_if_window_closed(task);
    kirk_qobuz_client_return_if_cancelled(self, task);

    GBytes* bytes =
        soup_session_send_and_read_finish(self->session, result, nullptr);

    if (!bytes) {
        self->status = KirkQobuzClientStatus::failed_connection;
        kirk_qobuz_client_return_result(task);
        return;
    }

    kirk_qobuz_client_find_the_bundle_url(task, bytes);
}

static void kirk_qobuz_client_fetch_the_play_host_page(GTask* task) {
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    const gchar* const url = QOBUZ_SCHEME QOBUZ_PLAY_HOST;
    SoupMessage* const msg = soup_message_new(SOUP_METHOD_GET, url);

    if (!msg) {
        self->status = KirkQobuzClientStatus::invalid_message;
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
    auto qobuz_client = new KirkQobuzClient;

    qobuz_client->session = soup_session_new();

    GTask* task = g_task_new(source_object, cancellable, callback, nullptr);
    g_task_set_check_cancellable(task, false);
    g_task_set_task_data(
        task,
        qobuz_client,
        reinterpret_cast<GDestroyNotify>(kirk_qobuz_client_free)
    );

    kirk_qobuz_client_fetch_the_play_host_page(task);
}
