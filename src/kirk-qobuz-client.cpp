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
    missing_token,
    invalid_message,
    failed_connection,
    failed_authorization,
    successful_authorization
};

struct KirkQobuzClient {
    SoupSession* session;

    gchar* app_id;
    gchar* token;

    KirkQobuzClientStatus status;
};

#define KIRK_QOBUZ_CLIENT(o) (static_cast<KirkQobuzClient*>(o))

static void kirk_qobuz_client_free(KirkQobuzClient* self) {
    g_object_unref(self->session);
    secret_password_free(self->token);
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
    case KirkQobuzClientStatus::missing_token:
        message = "Qobuz: coudln't lookup the token!";
        break;
    case KirkQobuzClientStatus::invalid_message:
        message = "Qobuz: invalid message!";
        break;
    case KirkQobuzClientStatus::failed_connection:
        message = "Qobuz: couldn't connect to the server!";
        break;
    case KirkQobuzClientStatus::failed_authorization:
        message = "Qobuz: authorization failed!";
        break;
    case KirkQobuzClientStatus::successful_authorization:
        message = "Qobuz: successful authorization!";
        break;
    default:
        message = "Qobuz: unknown error!";
        break;
    }

    g_task_return_pointer(task, const_cast<gchar*>(message), nullptr);
    g_object_unref(task);
}

#define kirk_qobuz_client_return_if_cancelled(self, task)                      \
    if (g_cancellable_is_cancelled(g_task_get_cancellable(task))) {            \
        (self)->status = KirkQobuzClientStatus::cancelled;                     \
        kirk_qobuz_client_return_result(task);                                 \
        return;                                                                \
    }

static void kirk_qobuz_client_send_authorization_request_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

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

#define QOBUZ_SCHEME "https://"
#define QOBUZ_HOST "www.qobuz.com"

#define QOBUZ_API_PATH "/api.json/0.2"
#define QOBUZ_LOGIN_PATH QOBUZ_API_PATH "/user/login"

static void kirk_qobuz_client_send_authorization_request(GTask* task) {
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    const gchar* const uri = QOBUZ_SCHEME QOBUZ_HOST QOBUZ_LOGIN_PATH;
    SoupMessage* const msg = soup_message_new(SOUP_METHOD_GET, uri);

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

static void kirk_qobuz_client_lookup_token_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    GTask* task = G_TASK(user_data);
    KirkQobuzClient* const self = KIRK_QOBUZ_CLIENT(g_task_get_task_data(task));

    kirk_qobuz_client_return_if_cancelled(self, task);

    gchar* token = kirk_secret_schema_lookup_password_finish(result, nullptr);

    if (!token || !token[0]) {
        self->status = KirkQobuzClientStatus::missing_token;
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
    auto qobuz_client = new KirkQobuzClient;

    qobuz_client->session = soup_session_new();
    qobuz_client->app_id = const_cast<gchar*>("950096963");

    GTask* task = g_task_new(nullptr, cancellable, callback, user_data);
    g_task_set_check_cancellable(task, false);
    g_task_set_task_data(
        task,
        qobuz_client,
        reinterpret_cast<GDestroyNotify>(kirk_qobuz_client_free)
    );

    kirk_qobuz_client_lookup_token(task);
}
