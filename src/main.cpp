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
#include "src/kirk-application.h"

#include <adwaita.h>
#include <filesystem>

// Necessary for debug builds when the GSettings schema is not installed yet
static void try_override_schema_dir() {
    std::filesystem::path process_cwd_path =
        std::filesystem::canonical("/proc/self/cwd");
    std::filesystem::path gsettings_schema_dir = process_cwd_path / "data";
    setenv("GSETTINGS_SCHEMA_DIR", gsettings_schema_dir.c_str(), true);
}

int main(int argc, char** argv) {
#ifdef APP_DEBUG_MODE_ON
    try_override_schema_dir();
#endif
    return g_application_run(G_APPLICATION(kirk_application_new()), argc, argv);
}
