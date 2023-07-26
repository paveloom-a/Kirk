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

#include <adwaita.h>

struct _KirkApplicationWindow {
    AdwApplicationWindow parent;

    GSettings *settings;

    GtkWidget greet_button;
    GtkWidget quit_button;
};

G_DEFINE_TYPE(
    KirkApplicationWindow,
    kirk_application_window,
    ADW_TYPE_APPLICATION_WINDOW
)

static void kirk_application_window_init(KirkApplicationWindow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));

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

static void greet_button_clicked(GtkButton *button, gpointer user_data) {
    g_print("Hello, world!\n");
}

static void kirk_application_window_dispose(GObject *object) {
    KirkApplicationWindow *self = KIRK_APPLICATION_WINDOW(object);

    g_clear_object(&self->settings);

    gtk_widget_dispose_template(GTK_WIDGET(self), KIRK_TYPE_APPLICATION_WINDOW);

    G_OBJECT_CLASS(kirk_application_window_parent_class)->dispose(object);
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

    gtk_widget_class_bind_template_child(
        widget_class,
        KirkApplicationWindow,
        greet_button
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        KirkApplicationWindow,
        quit_button
    );

    gtk_widget_class_bind_template_callback(widget_class, greet_button_clicked);
    gtk_widget_class_bind_template_callback(widget_class, gtk_window_destroy);
}

KirkApplicationWindow *kirk_application_window_new(KirkApplication *app) {
    return g_object_new(KIRK_TYPE_APPLICATION_WINDOW, "application", app, NULL);
}
