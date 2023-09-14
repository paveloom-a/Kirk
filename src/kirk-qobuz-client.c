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

#include <adwaita.h>
#include <gio/gnetworking.h>
#include <glibconfig.h>
#include <libsecret/secret.h>
#include <stdio.h>

typedef struct {
    gchar *app_id;
    gchar *user_id;
    gchar *token;

    GSocketConnection *connection;
    gchar *request;
    gchar response[QOBUZ_BUFFER_SIZE];
    guint16 status_code;
} KirkQobuzClient;

#define kirk_qobuz_client_return_if_cancelled(self, task)                      \
    if (g_cancellable_is_cancelled(g_task_get_cancellable(task))) {            \
        self->status_code = 0;                                                 \
        kirk_qobuz_client_return_result(task);                                 \
        return;                                                                \
    }

static void kirk_qobuz_client_free(KirkQobuzClient *self) {
    g_free(self->user_id);
    secret_password_free(self->token);
    g_clear_object(&self->connection);
    g_free(self->request);
    g_free(self);
}

void kirk_qobuz_client_result_free(KirkQobuzClientResult *self) {
    g_free(self);
}

static void kirk_qobuz_client_return_result(GTask *task) {
    const KirkQobuzClient *self = g_task_get_task_data(task);

    KirkQobuzClientResult *client_result = g_new0(KirkQobuzClientResult, 1);
    client_result->status_code = self->status_code;

    switch (client_result->status_code) {
    case 0:
        client_result->message = "Qobuz: operation was cancelled!";
        break;
    case 1:
        client_result->message = "Qobuz: coudln't lookup the token!";
        break;
    case 2:
        client_result->message = "Qobuz: coudln't connect to the host!";
        break;
    case 3:
        client_result->message = "Qobuz: couldn't send the login request!";
        break;
    case 4:
        client_result->message = "Qobuz: couldn't read the response!";
        break;
    case 5:
        client_result->message = "Qobuz: couldn't close the stream!";
        break;
    case 200:
        client_result->message = "Qobuz: successful authorization!";
        break;
    case 401:
        client_result->message = "Qobuz: authorization failed!";
        break;
    default:
        client_result->message = "Qobuz: unknown error!";
        break;
    }

    g_task_return_pointer(
        task,
        client_result,
        (GDestroyNotify)kirk_qobuz_client_result_free
    );

    g_object_unref(task);
}

static void kirk_qobuz_client_close_connection_finish(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    GIOStream *stream = G_IO_STREAM(source_object);
    GTask *task = G_TASK(user_data);
    KirkQobuzClient *self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_cancelled(self, task);

    gboolean success = g_io_stream_close_finish(stream, result, NULL);

    if (!success) {
        self->status_code = 5;
        kirk_qobuz_client_return_result(task);
        return;
    }

    kirk_qobuz_client_return_result(task);
}

static void kirk_qobuz_client_close_connection(GTask *task) {
    KirkQobuzClient *self = g_task_get_task_data(task);

    g_io_stream_close_async(
        G_IO_STREAM(self->connection),
        G_PRIORITY_DEFAULT,
        g_task_get_cancellable(task),
        kirk_qobuz_client_close_connection_finish,
        task
    );
}

static void kirk_qobuz_client_read_response_finish(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    GInputStream *input_stream = G_INPUT_STREAM(source_object);
    GTask *task = G_TASK(user_data);
    KirkQobuzClient *self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_cancelled(self, task);

    gssize bytes_read = g_input_stream_read_finish(input_stream, result, NULL);

    if (bytes_read < 0) {
        self->status_code = 4;
        kirk_qobuz_client_return_result(task);
        return;
    }
}

static void kirk_qobuz_client_read_response(
    GAsyncReadyCallback callback,
    GTask *task
) {
    KirkQobuzClient *self = g_task_get_task_data(task);

    GInputStream *input_stream =
        g_io_stream_get_input_stream(G_IO_STREAM(self->connection));

    memset(self->response, 0, sizeof(self->response));
    g_input_stream_read_async(
        input_stream,
        self->response,
        QOBUZ_BUFFER_SIZE - 1,
        G_PRIORITY_DEFAULT,
        g_task_get_cancellable(task),
        callback,
        task
    );
}

static void kirk_qobuz_client_read_login_response_finish(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    GTask *task = G_TASK(user_data);
    KirkQobuzClient *self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_cancelled(self, task);

    kirk_qobuz_client_read_response_finish(source_object, result, user_data);

    guint16 status_code = 0;
    sscanf(self->response, "HTTP/1.1 %hu", &status_code);
    self->status_code = status_code;

    kirk_qobuz_client_close_connection(task);
}

static void kirk_qobuz_client_read_login_response(GTask *task) {
    kirk_qobuz_client_read_response(
        kirk_qobuz_client_read_login_response_finish,
        task
    );
}

static void kirk_qobuz_client_send_login_request_finish(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    GOutputStream *output_stream = G_OUTPUT_STREAM(source_object);
    GTask *task = G_TASK(user_data);
    KirkQobuzClient *self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_cancelled(self, task);

    gboolean success =
        g_output_stream_write_all_finish(output_stream, result, NULL, NULL);

    if (!success) {
        self->status_code = 3;
        kirk_qobuz_client_return_result(task);
        return;
    }

    kirk_qobuz_client_read_login_response(task);
}

static void kirk_qobuz_client_send_login_request(GTask *task) {
    KirkQobuzClient *self = g_task_get_task_data(task);

    GOutputStream *output_stream =
        g_io_stream_get_output_stream(G_IO_STREAM(self->connection));

    self->request = g_strconcat(
        "GET " QOBUZ_LOGIN_PATH "?app_id=",
        self->app_id,
        "&user_id=",
        self->user_id,
        "&user_auth_token=",
        self->token,
        " HTTP/1.1\r\n"
        "Host: " QOBUZ_HOST "\r\n"
        "Accept: application/json\r\n"
        "\r\n",
        NULL
    );

    g_output_stream_write_all_async(
        output_stream,
        self->request,
        strlen(self->request),
        G_PRIORITY_DEFAULT,
        g_task_get_cancellable(task),
        kirk_qobuz_client_send_login_request_finish,
        task
    );
}

static void kirk_qobuz_client_connect_finish(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    GSocketClient *socket_client = G_SOCKET_CLIENT(source_object);
    GTask *task = G_TASK(user_data);
    KirkQobuzClient *self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_cancelled(self, task);

    self->connection =
        g_socket_client_connect_to_host_finish(socket_client, result, NULL);

    if (self->connection == NULL) {
        self->status_code = 2;
        kirk_qobuz_client_return_result(task);
        return;
    }

    g_tcp_connection_set_graceful_disconnect(
        G_TCP_CONNECTION(self->connection),
        TRUE
    );

    kirk_qobuz_client_send_login_request(task);
}

static void kirk_qobuz_client_connect(GTask *task) {
    g_autoptr(GSocketClient) socket_client = g_socket_client_new();

    g_socket_client_connect_to_host_async(
        socket_client,
        QOBUZ_HOST,
        80,
        g_task_get_cancellable(task),
        kirk_qobuz_client_connect_finish,
        task
    );
}

static void kirk_qobuz_client_lookup_token_finish(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    GTask *task = G_TASK(user_data);
    KirkQobuzClient *self = g_task_get_task_data(task);

    kirk_qobuz_client_return_if_cancelled(self, task);

    gchar *token = kirk_secret_schema_lookup_password_finish(result, NULL);

    if (token == NULL || token[0] == '\0') {
        self->status_code = 1;
        kirk_qobuz_client_return_result(task);
        return;
    }

    self->token = token;

    kirk_qobuz_client_connect(task);
}

static void kirk_qobuz_client_lookup_token(GTask *task) {
    kirk_secret_schema_lookup_password(
        "qobuz",
        g_task_get_cancellable(task),
        kirk_qobuz_client_lookup_token_finish,
        task
    );
}

void kirk_qobuz_client_send_authorization_request(
    GSettings *settings,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    KirkQobuzClient *qobuz_client = g_malloc0(sizeof(KirkQobuzClient));
    qobuz_client->app_id = "950096963";
    qobuz_client->user_id = g_settings_get_string(settings, "qobuz-user-id");

    GTask *task = g_task_new(NULL, cancellable, callback, user_data);
    g_task_set_check_cancellable(task, FALSE);
    g_task_set_task_data(
        task,
        qobuz_client,
        (GDestroyNotify)kirk_qobuz_client_free
    );

    kirk_qobuz_client_lookup_token(task);
}
