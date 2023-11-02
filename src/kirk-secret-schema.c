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

#include "src/kirk-secret-schema.h"

#include "include/config.h"

#include <libsecret/secret.h>

const SecretSchema* kirk_secret_schema_get_type() {
    static const SecretSchema schema = {
        .name = APP_ID,
        .flags = SECRET_SCHEMA_NONE,
        .attributes = {
            {"schema", SECRET_SCHEMA_ATTRIBUTE_STRING},
            {"service", SECRET_SCHEMA_ATTRIBUTE_STRING},
            {"parameter", SECRET_SCHEMA_ATTRIBUTE_STRING},
            {NULL, SECRET_SCHEMA_ATTRIBUTE_STRING},
        }};
    return &schema;
}

void kirk_secret_schema_store_password_finish(
    GAsyncResult* result,
    GError** error
) {
    secret_password_store_finish(result, error);
}

void kirk_secret_schema_store_password_callback(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    kirk_secret_schema_store_password_finish(result, NULL);
}

void kirk_secret_schema_store_password(
    const gchar* label,
    const gchar* password,
    const gchar* service,
    const gchar* parameter,
    GAsyncReadyCallback callback
) {
    secret_password_store(
        KIRK_SECRET_SCHEMA,
        SECRET_COLLECTION_DEFAULT,
        label,
        password,
        NULL,
        callback,
        NULL,
        "schema",
        APP_ID,
        "service",
        service,
        "parameter",
        parameter,
        NULL
    );
}

void kirk_secret_schema_store_qobuz_token(const gchar* token) {
    kirk_secret_schema_store_password(
        "Kirk: Qobuz token",
        token,
        "qobuz",
        "token",
        kirk_secret_schema_store_password_callback
    );
}

void kirk_secret_schema_store_qobuz_app_id(const gchar* app_id) {
    kirk_secret_schema_store_password(
        "Kirk: Qobuz application ID",
        app_id,
        "qobuz",
        "app_id",
        kirk_secret_schema_store_password_callback
    );
}

gchar* kirk_secret_schema_lookup_password_finish(
    GAsyncResult* result,
    GError** error
) {
    return secret_password_lookup_finish(result, error);
}

void kirk_secret_schema_lookup_password(
    const gchar* service,
    const gchar* parameter,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    secret_password_lookup(
        KIRK_SECRET_SCHEMA,
        cancellable,
        callback,
        user_data,
        "schema",
        APP_ID,
        "service",
        service,
        "parameter",
        parameter,
        NULL
    );
}
