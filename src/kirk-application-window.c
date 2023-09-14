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

#include "src/kirk-application-window.h"

#include "include/config.h"
#include "src/kirk-add-release-window.h"
#include "src/kirk-preferences-window.h"

#include <adwaita.h>

struct _KirkApplicationWindow {
    AdwApplicationWindow parent;

    GSettings *settings;
};

G_DEFINE_TYPE(
    KirkApplicationWindow,
    kirk_application_window,
    ADW_TYPE_APPLICATION_WINDOW
)

static void prepare_settings(KirkApplicationWindow *self) {
    self->settings = g_settings_new(APP_ID);

    g_settings_bind(
        self->settings,
        "window-width",
        G_OBJECT(self),
        "default-width",
        G_SETTINGS_BIND_DEFAULT
    );
    g_settings_bind(
        self->settings,
        "window-height",
        G_OBJECT(self),
        "default-height",
        G_SETTINGS_BIND_DEFAULT
    );
    g_settings_bind(
        self->settings,
        "window-maximized",
        G_OBJECT(self),
        "maximized",
        G_SETTINGS_BIND_DEFAULT
    );
}

static void kirk_application_window_init(KirkApplicationWindow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));

    prepare_settings(self);
}

static void kirk_application_window_dispose(GObject *object) {
    KirkApplicationWindow *self = KIRK_APPLICATION_WINDOW(object);

    g_clear_object(&self->settings);

    gtk_widget_dispose_template(GTK_WIDGET(self), KIRK_TYPE_APPLICATION_WINDOW);

    G_OBJECT_CLASS(kirk_application_window_parent_class)->dispose(object);
}

static void add_release(
    GtkWidget *widget,
    const gchar *action_name,
    GVariant *parameter
) {
    const KirkApplicationWindow *self = KIRK_APPLICATION_WINDOW(widget);
    KirkAddReleaseWindow *add_release_win =
        kirk_add_release_window_new(KIRK_DEFAULT_APPLICATION, self);
    gtk_window_present(GTK_WINDOW(add_release_win));
}

static void open_preferences(
    GtkWidget *widget,
    const gchar *action_name,
    GVariant *parameter
) {
    const KirkApplicationWindow *self = KIRK_APPLICATION_WINDOW(widget);

    KirkPreferencesWindow *preferences_win =
        kirk_preferences_window_new(KIRK_DEFAULT_APPLICATION, self);
    gtk_window_present(GTK_WINDOW(preferences_win));
}

static void kirk_application_window_class_init(  //
    KirkApplicationWindowClass *klass
) {
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(
        widget_class,
        APP_RESOURCES_PATH "gtk/kirk-application-window.ui"
    );

    G_OBJECT_CLASS(klass)->dispose = kirk_application_window_dispose;

    gtk_widget_class_install_action(
        widget_class,
        "win.add-release",
        NULL,
        add_release
    );
    gtk_widget_class_install_action(
        widget_class,
        "win.show-preferences",
        NULL,
        open_preferences
    );

    gtk_widget_class_add_binding_action(
        widget_class,
        GDK_KEY_A,
        GDK_CONTROL_MASK,
        "win.add-release",
        NULL
    );
    gtk_widget_class_add_binding_action(
        widget_class,
        GDK_KEY_comma,
        GDK_CONTROL_MASK,
        "win.show-preferences",
        NULL
    );
}

KirkApplicationWindow *kirk_application_window_new(const KirkApplication *app) {
    return g_object_new(KIRK_TYPE_APPLICATION_WINDOW, "application", app, NULL);
}
