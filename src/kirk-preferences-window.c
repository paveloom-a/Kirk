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

#include "src/kirk-preferences-window.h"

#include "include/config.h"

#include <adwaita.h>

struct _KirkPreferencesWindow {
    AdwPreferencesWindow parent;

    GSettings *settings;

    GtkWidget *destination_folder_entry_row;
};

G_DEFINE_TYPE(
    KirkPreferencesWindow,
    kirk_preferences_window,
    ADW_TYPE_PREFERENCES_WINDOW
)

static void select_destination_folder_finish(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    KirkPreferencesWindow *self = KIRK_PREFERENCES_WINDOW(user_data);
    GtkFileDialog *file_dialog = GTK_FILE_DIALOG(source_object);

    g_autoptr(GFile) file =
        gtk_file_dialog_select_folder_finish(file_dialog, result, NULL);

    if (file == NULL) {
        return;
    }

    const gchar *destination_folder_path = g_file_peek_path(file);
    g_settings_set_string(
        self->settings,
        "destination-folder-path",
        destination_folder_path
    );
}

static void select_destination_folder(GtkButton *button, gpointer self) {
    g_autoptr(GtkFileDialog) file_dialog = gtk_file_dialog_new();
    gtk_file_dialog_select_folder(
        file_dialog,
        GTK_WINDOW(self),
        NULL,
        select_destination_folder_finish,
        self
    );
}

static void kirk_preferences_window_init(KirkPreferencesWindow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));

    self->settings = g_settings_new(APP_ID);

    g_settings_bind(
        self->settings,
        "destination-folder-path",
        G_OBJECT(self->destination_folder_entry_row),
        "text",
        G_SETTINGS_BIND_DEFAULT
    );
}

static void kirk_preferences_window_dispose(GObject *object) {
    KirkPreferencesWindow *self = KIRK_PREFERENCES_WINDOW(object);

    g_clear_object(&self->settings);

    gtk_widget_dispose_template(GTK_WIDGET(self), KIRK_TYPE_PREFERENCES_WINDOW);

    G_OBJECT_CLASS(kirk_preferences_window_parent_class)->dispose(object);
}

static void kirk_preferences_window_class_init(KirkPreferencesWindowClass *klass
) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(
        widget_class,
        APP_RESOURCES_PATH "gtk/kirk-preferences-window.ui"
    );

    object_class->dispose = kirk_preferences_window_dispose;

    gtk_widget_class_bind_template_child(
        widget_class,
        KirkPreferencesWindow,
        destination_folder_entry_row
    );

    gtk_widget_class_bind_template_callback(
        widget_class,
        select_destination_folder
    );
}

KirkPreferencesWindow *kirk_preferences_window_new(
    KirkApplication *app,
    KirkApplicationWindow *app_win
) {
    return g_object_new(
        KIRK_TYPE_PREFERENCES_WINDOW,
        "application",
        app,
        "transient-for",
        app_win,
        NULL
    );
}
