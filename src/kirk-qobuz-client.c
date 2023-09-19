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
    KIRK_QOBUZ_CLIENT_STATE_CANCELLED,
    KIRK_QOBUZ_CLIENT_STATE_MISSING_TOKEN,
    KIRK_QOBUZ_CLIENT_STATE_FAILED_CONNECTION,
    KIRK_QOBUZ_CLIENT_STATE_FAILED_AUTHORIZATION,
    KIRK_QOBUZ_CLIENT_STATE_SUCCESSFUL_AUTHORIZATION
} KirkQobuzClientStatus;

typedef struct {
    SoupSession* session;

    gchar* app_id;
    gchar* user_id;
    gchar* token;

    KirkQobuzClientStatus status;
} KirkQobuzClient;

static void kirk_qobuz_client_free(KirkQobuzClient* self) {
    g_object_unref(self->session);
    g_free(self->user_id);
    secret_password_free(self->token);
    g_free(self);
}

static void kirk_qobuz_client_return_result(GTask* task) {
    const KirkQobuzClient* self = g_task_get_task_data(task);

    gchar* message;
    switch (self->status) {
    case KIRK_QOBUZ_CLIENT_STATE_CANCELLED:
        message = "Qobuz: operation was cancelled!";
        break;
    case KIRK_QOBUZ_CLIENT_STATE_MISSING_TOKEN:
        message = "Qobuz: coudln't lookup the token!";
        break;
    case KIRK_QOBUZ_CLIENT_STATE_FAILED_CONNECTION:
        message = "Qobuz: couldn't connect to the server!";
        break;
    case KIRK_QOBUZ_CLIENT_STATE_FAILED_AUTHORIZATION:
        message = "Qobuz: authorization failed!";
        break;
    case KIRK_QOBUZ_CLIENT_STATE_SUCCESSFUL_AUTHORIZATION:
        message = "Qobuz: successful authorization!";
        break;
    default:
        message = "Qobuz: unknown error!";
        break;
    }

    g_task_return_pointer(task, message, NULL);
    g_object_unref(task);
}

#define kirk_qobuz_client_return_if_cancelled(self, task)                      \
    if (g_cancellable_is_cancelled(g_task_get_cancellable(task))) {            \
        self->status = KIRK_QOBUZ_CLIENT_STATE_CANCELLED;                      \
        kirk_qobuz_client_return_result(task);                                 \
        return;                                                                \
    }

static void kirk_qobuz_client_send_authorization_request_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_cancelled(self, task);

    soup_session_send_finish(self->session, result, NULL);
    SoupMessage* msg =
        soup_session_get_async_result_message(self->session, result);

    if (msg == NULL) {
        self->status = KIRK_QOBUZ_CLIENT_STATE_FAILED_CONNECTION;
        kirk_qobuz_client_return_result(task);
        return;
    }

    SoupStatus status = soup_message_get_status(msg);
    if (status == SOUP_STATUS_OK) {
        self->status = KIRK_QOBUZ_CLIENT_STATE_SUCCESSFUL_AUTHORIZATION;
    } else {
        self->status = KIRK_QOBUZ_CLIENT_STATE_FAILED_AUTHORIZATION;
    }

    kirk_qobuz_client_return_result(task);
}

#define QOBUZ_SCHEME "http://"
#define QOBUZ_HOST "www.qobuz.com"

#define QOBUZ_API_PATH "/api.json/0.2"
#define QOBUZ_LOGIN_PATH QOBUZ_API_PATH "/user/login"

static void kirk_qobuz_client_send_authorization_request(GTask* task) {
    const KirkQobuzClient* self = g_task_get_task_data(task);

    const g_autofree gchar* uri = kirk_uri_new(
        QOBUZ_SCHEME,
        QOBUZ_HOST,
        QOBUZ_LOGIN_PATH,
        kirk_uri_key_value("app_id", self->app_id),
        kirk_uri_key_value("user_id", self->user_id),
        kirk_uri_key_value("user_auth_token", self->token),
        NULL
    );
    SoupMessage* msg = soup_message_new(SOUP_METHOD_GET, uri);

    soup_session_send_async(
        self->session,
        msg,
        G_PRIORITY_DEFAULT,
        g_task_get_cancellable(task),
        kirk_qobuz_client_send_authorization_request_finish,
        task
    );
}

static void kirk_qobuz_client_lookup_token_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_cancelled(self, task);

    gchar* token = kirk_secret_schema_lookup_password_finish(result, NULL);

    if (token == NULL || token[0] == '\0') {
        self->status = KIRK_QOBUZ_CLIENT_STATE_MISSING_TOKEN;
        kirk_qobuz_client_return_result(task);
        return;
    }

    self->token = token;

    kirk_qobuz_client_send_authorization_request(task);
}

static void kirk_qobuz_client_lookup_token(GTask* task) {
    kirk_secret_schema_lookup_password(
        "qobuz",
        g_task_get_cancellable(task),
        kirk_qobuz_client_lookup_token_finish,
        task
    );
}

void kirk_qobuz_client_try_to_authorize(
    GSettings* settings,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    KirkQobuzClient* qobuz_client = g_malloc0(sizeof(KirkQobuzClient));

    qobuz_client->session = soup_session_new();

    qobuz_client->app_id = "950096963";
    qobuz_client->user_id = g_settings_get_string(settings, "qobuz-user-id");

    GTask* task = g_task_new(NULL, cancellable, callback, user_data);
    g_task_set_check_cancellable(task, FALSE);
    g_task_set_task_data(
        task,
        qobuz_client,
        (GDestroyNotify)kirk_qobuz_client_free
    );

    kirk_qobuz_client_lookup_token(task);
}
