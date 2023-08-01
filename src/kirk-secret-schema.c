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

#include "include/config.h"

#include <libsecret/secret.h>

const SecretSchema *kirk_secret_schema_get_type() {
    static const SecretSchema schema = {
        .name = APP_ID,
        .flags = SECRET_SCHEMA_NONE,
        .attributes = {
            {"schema", SECRET_SCHEMA_ATTRIBUTE_STRING},
            {"service", SECRET_SCHEMA_ATTRIBUTE_STRING},
            {NULL, 0},
        }};
    return &schema;
}
