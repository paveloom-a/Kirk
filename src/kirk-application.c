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

#include <adwaita.h>

struct _KirkApplication {
    AdwApplication parent;

    GSettings *settings;
};

G_DEFINE_TYPE(KirkApplication, kirk_application, ADW_TYPE_APPLICATION)

static void kirk_application_init(KirkApplication *self) {
    self->settings = g_settings_new(APP_ID);
}

static void quit(
    GSimpleAction *action,
    GVariant *parameter,
    gpointer user_data
) {
    g_application_quit(G_APPLICATION(user_data));
}

static void prepare_actions(KirkApplication *self) {
    GtkApplication *app = GTK_APPLICATION(self);

    const GActionEntry entries[] = {
        {.name = "quit", .activate = quit},
    };
    g_action_map_add_action_entries(
        G_ACTION_MAP(self),
        entries,
        G_N_ELEMENTS(entries),
        self
    );

    gtk_application_set_accels_for_action(
        app,
        "app.quit",
        (const gchar *const[]){"<primary>q", NULL}
    );
}

static void prepare_settings(KirkApplication *self) {
    const g_autofree gchar *destination_folder_path =
        g_settings_get_string(self->settings, "destination-folder-path");

    if (destination_folder_path != NULL && destination_folder_path[0] != '\0') {
        return;
    }

    const gchar *music_directory_path =
        g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);

    if (music_directory_path != NULL) {
        g_settings_set_string(
            self->settings,
            "destination-folder-path",
            music_directory_path
        );
    }
}

static void kirk_application_startup(GApplication *app) {
    KirkApplication *self = KIRK_APPLICATION(app);

    prepare_actions(self);
    prepare_settings(self);

    G_APPLICATION_CLASS(kirk_application_parent_class)->startup(app);
}

static void kirk_application_activate(GApplication *app) {
    KirkApplication *self = KIRK_APPLICATION(app);

    KirkApplicationWindow *win = kirk_application_window_new(self);
    gtk_window_present(GTK_WINDOW(win));
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
