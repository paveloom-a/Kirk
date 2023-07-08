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

#include "src/groovy-application-window.h"

#include "include/config.h"

#include <gtk/gtk.h>

struct _GroovyApplicationWindow {
    GtkApplicationWindow parent;

    GtkWidget greet_button;
    GtkWidget quit_button;
};

G_DEFINE_TYPE(
    GroovyApplicationWindow,
    groovy_application_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void groovy_application_window_init(GroovyApplicationWindow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void greet_button_clicked(GtkButton *button, gpointer user_data) {
    g_print("Hello, world!\n");
}

static void groovy_application_window_dispose(GObject *gobject) {
    GroovyApplicationWindow *self = GROOVY_APPLICATION_WINDOW(gobject);

    gtk_widget_dispose_template(
        GTK_WIDGET(self),
        GROOVY_TYPE_APPLICATION_WINDOW
    );

    G_OBJECT_CLASS(groovy_application_window_parent_class)->dispose(gobject);
}

static void groovy_application_window_class_init(
    GroovyApplicationWindowClass *klass
) {
    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(klass),
        APP_RESOURCES_PATH "gtk/groovy-application-window.ui"
    );

    G_OBJECT_CLASS(klass)->dispose = groovy_application_window_dispose;

    gtk_widget_class_bind_template_child(
        GTK_WIDGET_CLASS(klass),
        GroovyApplicationWindow,
        greet_button
    );
    gtk_widget_class_bind_template_child(
        GTK_WIDGET_CLASS(klass),
        GroovyApplicationWindow,
        quit_button
    );

    gtk_widget_class_bind_template_callback(
        GTK_WIDGET_CLASS(klass),
        greet_button_clicked
    );
    gtk_widget_class_bind_template_callback(
        GTK_WIDGET_CLASS(klass),
        gtk_window_destroy
    );
}

GroovyApplicationWindow *groovy_application_window_new(GroovyApplication *app) {
    return g_object_new(
        GROOVY_TYPE_APPLICATION_WINDOW,
        "application",
        app,
        NULL
    );
}
