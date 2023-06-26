// Groovy
// Copyright (C) 2023  Pavel Sobolev
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

#include <gtk/gtk.h>

static void print_hello(GtkWidget *widget, gpointer data) {
    g_print("Hello, world!\n");
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_resource(
        builder,
        "/paveloom/apps/Groovy/gtk/builder.ui",
        NULL
    );

    GObject *window = gtk_builder_get_object(builder, "window");
    gtk_window_set_application(GTK_WINDOW(window), app);

    GObject *button_1 = gtk_builder_get_object(builder, "button_1");
    g_signal_connect(button_1, "clicked", G_CALLBACK(print_hello), NULL);

    GObject *button_2 = gtk_builder_get_object(builder, "button_2");
    g_signal_connect(button_2, "clicked", G_CALLBACK(print_hello), NULL);

    GObject *quit_button = gtk_builder_get_object(builder, "quit_button");
    g_signal_connect_swapped(
        quit_button,
        "clicked",
        G_CALLBACK(gtk_window_destroy),
        window
    );

    g_object_unref(builder);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new(
        "paveloom.apps.Groovy",
        G_APPLICATION_DEFAULT_FLAGS
    );
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
