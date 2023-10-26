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

#include "src/kirk-add-release-window.h"

#include "include/config.h"

#include <adwaita.h>

struct _KirkAddReleaseWindow {
    AdwWindow parent;

    GSettings* settings;
};

G_DEFINE_TYPE(KirkAddReleaseWindow, kirk_add_release_window, ADW_TYPE_WINDOW)

static void kirk_add_release_window_init(KirkAddReleaseWindow* self) {
    gtk_widget_init_template(GTK_WIDGET(self));

    self->settings = g_settings_new(APP_ID);
}

static void kirk_add_release_window_dispose(GObject* object) {
    KirkAddReleaseWindow* self = KIRK_ADD_RELEASE_WINDOW(object);

    g_clear_object(&self->settings);

    gtk_widget_dispose_template(GTK_WIDGET(self), KIRK_TYPE_ADD_RELEASE_WINDOW);

    G_OBJECT_CLASS(kirk_add_release_window_parent_class)->dispose(object);
}

static void kirk_add_release_window_class_init(KirkAddReleaseWindowClass* klass
) {
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(
        widget_class,
        APP_RESOURCES_PATH "gtk/kirk-add-release-window.ui"
    );

    object_class->dispose = kirk_add_release_window_dispose;
}

KirkAddReleaseWindow* kirk_add_release_window_new(
    const KirkApplication* app,
    const KirkApplicationWindow* app_win
) {
    return static_cast<KirkAddReleaseWindow*>(g_object_new(
        KIRK_TYPE_ADD_RELEASE_WINDOW,
        "application",
        app,
        "transient-for",
        app_win,
        NULL
    ));
}
