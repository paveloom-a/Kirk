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

#include "include/config.h"

#include <gtk/gtk.h>

static void print_hello(GtkWidget *widget, gpointer data) {
    g_print("Hello, world!\n");
}

static void activate(GtkApplication *app, gpointer data) {
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_resource(builder, APP_PATH "ui/main.ui", NULL);

    GObject *window = gtk_builder_get_object(builder, "window");
    gtk_window_set_application(GTK_WINDOW(window), app);

    GObject *great_button = gtk_builder_get_object(builder, "great_button");
    g_signal_connect(great_button, "clicked", G_CALLBACK(print_hello), NULL);

    GObject *quit_button = gtk_builder_get_object(builder, "quit_button");
    g_signal_connect_swapped(
        quit_button,
        "clicked",
        G_CALLBACK(gtk_window_destroy),
        window
    );

    gtk_window_present(GTK_WINDOW(window));

    g_object_unref(builder);
}

int main(int argc, char **argv) {
    GtkApplication *app =
        gtk_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}
