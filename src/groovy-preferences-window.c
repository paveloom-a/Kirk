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

#include "src/groovy-preferences-window.h"

#include "include/config.h"

#include <adwaita.h>

struct _GroovyPreferencesWindow {
    AdwPreferencesWindow parent;

    GSettings *settings;
};

G_DEFINE_TYPE(
    GroovyPreferencesWindow,
    groovy_preferences_window,
    ADW_TYPE_PREFERENCES_WINDOW
)

static void groovy_preferences_window_init(GroovyPreferencesWindow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));

    self->settings = g_settings_new(APP_ID);
}

static void groovy_preferences_window_dispose(GObject *object) {
    GroovyPreferencesWindow *self = GROOVY_PREFERENCES_WINDOW(object);

    g_clear_object(&self->settings);

    gtk_widget_dispose_template(
        GTK_WIDGET(self),
        GROOVY_TYPE_PREFERENCES_WINDOW
    );

    G_OBJECT_CLASS(groovy_preferences_window_parent_class)->dispose(object);
}

static void groovy_preferences_window_class_init(
    GroovyPreferencesWindowClass *klass
) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(
        widget_class,
        APP_RESOURCES_PATH "gtk/groovy-preferences-window.ui"
    );

    object_class->dispose = groovy_preferences_window_dispose;
}

GroovyPreferencesWindow *groovy_preferences_window_new(
    GroovyApplication *app,
    GroovyApplicationWindow *app_win
) {
    return g_object_new(
        GROOVY_TYPE_PREFERENCES_WINDOW,
        "application",
        app,
        "transient-for",
        app_win,
        NULL
    );
}
