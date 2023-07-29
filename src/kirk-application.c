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

#include "src/kirk-application.h"

#include "include/config.h"
#include "src/kirk-application-window.h"
#include "src/kirk-preferences-window.h"

#include <adwaita.h>

struct _KirkApplication {
    AdwApplication parent;

    GSettings *settings;
};

G_DEFINE_TYPE(KirkApplication, kirk_application, ADW_TYPE_APPLICATION)

static void kirk_application_init(KirkApplication *self) {
    self->settings = g_settings_new(APP_ID);
}

static void open_preferences(
    GSimpleAction *action,
    GVariant *parameter,
    gpointer user_data
) {
    KirkApplication *self = KIRK_APPLICATION(user_data);
    GtkWindow *win = gtk_application_get_active_window(GTK_APPLICATION(self));
    KirkPreferencesWindow *prefs_win =
        kirk_preferences_window_new(self, KIRK_APPLICATION_WINDOW(win));
    gtk_window_present(GTK_WINDOW(prefs_win));
}

static void quit_application(
    GSimpleAction *action,
    GVariant *parameter,
    gpointer user_data
) {
    g_application_quit(G_APPLICATION(user_data));
}

static void kirk_application_startup(GApplication *app) {
    KirkApplication *self = KIRK_APPLICATION(app);

    G_APPLICATION_CLASS(kirk_application_parent_class)->startup(app);

    const GActionEntry entries[] = {
        {.name = "preferences", .activate = open_preferences},
        {.name = "quit", .activate = quit_application},
    };
    g_action_map_add_action_entries(
        G_ACTION_MAP(self),
        entries,
        G_N_ELEMENTS(entries),
        self
    );

    const char *preferences_accels[] = {"<primary>comma", NULL};
    gtk_application_set_accels_for_action(
        GTK_APPLICATION(self),
        "app.preferences",
        preferences_accels
    );

    const char *quit_accels[] = {"<primary>q", NULL};
    gtk_application_set_accels_for_action(
        GTK_APPLICATION(self),
        "app.quit",
        quit_accels
    );
}

static void maybe_override_destination_folder_path(GSettings *settings) {
    g_autofree const gchar *destination_folder_path =
        g_settings_get_string(settings, "destination-folder-path");

    if (destination_folder_path != NULL && destination_folder_path[0] != '\0') {
        return;
    }

    const gchar *music_directory_path =
        g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);

    if (music_directory_path != NULL) {
        g_settings_set_string(
            settings,
            "destination-folder-path",
            music_directory_path
        );
    }
}

static void kirk_application_activate(GApplication *app) {
    KirkApplication *self = KIRK_APPLICATION(app);

    KirkApplicationWindow *win = kirk_application_window_new(self);
    gtk_window_present(GTK_WINDOW(win));

    maybe_override_destination_folder_path(self->settings);
}

static void kirk_application_shutdown(GApplication *app) {
    KirkApplication *self = KIRK_APPLICATION(app);

    g_clear_object(&self->settings);

    G_APPLICATION_CLASS(kirk_application_parent_class)->shutdown(app);
}

static void kirk_application_class_init(KirkApplicationClass *klass) {
    GApplicationClass *app_klass = G_APPLICATION_CLASS(klass);

    app_klass->startup = kirk_application_startup;
    app_klass->activate = kirk_application_activate;
    app_klass->shutdown = kirk_application_shutdown;
}

KirkApplication *kirk_application_new() {
    return g_object_new(
        KIRK_TYPE_APPLICATION,
        "application-id",
        APP_ID,
        "flags",
        G_APPLICATION_HANDLES_OPEN,
        NULL
    );
}
