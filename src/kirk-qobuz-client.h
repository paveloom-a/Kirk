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

#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define QOBUZ_HOST "www.qobuz.com"

#define QOBUZ_API_PATH "/api.json/0.2"
#define QOBUZ_LOGIN_PATH QOBUZ_API_PATH "/user/login"

#define QOBUZ_BUFFER_SIZE 1024

typedef struct {
    gint status_code;
    gchar *message;
} KirkQobuzClientResult;

void kirk_qobuz_client_result_free(KirkQobuzClientResult *self);

void kirk_qobuz_client_send_authorization_request(
    GSettings *settings,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
);

G_END_DECLS
