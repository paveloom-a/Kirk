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
            {nullptr, SECRET_SCHEMA_ATTRIBUTE_STRING},
        }};
    return &schema;
}

void kirk_secret_schema_store_password_finish(
    GAsyncResult* result,
    GError** error
) {
    secret_password_store_finish(result, error);
}

void kirk_secret_schema_store_password(
    const gchar* label,
    const gchar* password,
    const gchar* service,
    GAsyncReadyCallback callback
) {
    secret_password_store(
        KIRK_SECRET_SCHEMA,
        SECRET_COLLECTION_DEFAULT,
        label,
        password,
        nullptr,
        callback,
        nullptr,
        "schema",
        APP_ID,
        "service",
        service,
        NULL
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
        NULL
    );
}
