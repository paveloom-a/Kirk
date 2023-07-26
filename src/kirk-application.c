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
};

G_DEFINE_TYPE(KirkApplication, kirk_application, ADW_TYPE_APPLICATION)

static void kirk_application_init(KirkApplication *self) {}

static void preferences_activated(
    GSimpleAction *action,
    GVariant *parameter,
    gpointer user_data
) {
    KirkApplication *self = KIRK_APPLICATION(user_data);
    GtkWindow *win = gtk_application_get_active_window(GTK_APPLICATION(self));
    KirkPreferencesWindow *prefs =
        kirk_preferences_window_new(self, KIRK_APPLICATION_WINDOW(win));
    gtk_window_present(GTK_WINDOW(prefs));
}

static void quit_activated(
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
        {.name = "preferences", .activate = preferences_activated},
        {.name = "quit", .activate = quit_activated},
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

static void kirk_application_activate(GApplication *app) {
    KirkApplication *self = KIRK_APPLICATION(app);

    KirkApplicationWindow *win = kirk_application_window_new(self);
    gtk_window_present(GTK_WINDOW(win));
}

static void kirk_application_class_init(KirkApplicationClass *klass) {
    G_APPLICATION_CLASS(klass)->startup = kirk_application_startup;
    G_APPLICATION_CLASS(klass)->activate = kirk_application_activate;
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
