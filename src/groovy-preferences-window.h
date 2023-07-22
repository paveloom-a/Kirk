// Groovy
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

#include "src/groovy-application-window.h"

#include <adwaita.h>

G_BEGIN_DECLS

#define GROOVY_TYPE_PREFERENCES_WINDOW groovy_preferences_window_get_type()

G_DECLARE_FINAL_TYPE(
    GroovyPreferencesWindow,
    groovy_preferences_window,
    GROOVY,
    PREFERENCES_WINDOW,
    AdwPreferencesWindow
)

GroovyPreferencesWindow *groovy_preferences_window_new(
    GroovyApplication *app,
    GroovyApplicationWindow *app_win
);

G_END_DECLS
