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

#include "src/groovy-application.h"

#include "include/config.h"
#include "src/groovy-application-window.h"

#include <gtk/gtk.h>

struct _GroovyApplication {
    GtkApplication parent;
};

G_DEFINE_TYPE(GroovyApplication, groovy_application, GTK_TYPE_APPLICATION)

static void groovy_application_init(GroovyApplication *app) {}

static void quit_activated(
    GSimpleAction *action,
    GVariant *parameter,
    gpointer user_data
) {
    g_application_quit(G_APPLICATION(user_data));
}

static GActionEntry app_action_entries[] = {
    {.name = "quit", .activate = quit_activated},
};

static void groovy_application_startup(GApplication *app) {
    G_APPLICATION_CLASS(groovy_application_parent_class)->startup(app);

    g_action_map_add_action_entries(
        G_ACTION_MAP(app),
        app_action_entries,
        G_N_ELEMENTS(app_action_entries),
        app
    );

    const char *quit_accels[2] = {"<primary>q", NULL};
    gtk_application_set_accels_for_action(
        GTK_APPLICATION(app),
        "app.quit",
        quit_accels
    );
}

static void groovy_application_activate(GApplication *app) {
    GroovyApplicationWindow *win =
        groovy_application_window_new(GROOVY_APPLICATION(app));
    gtk_window_present(GTK_WINDOW(win));
}

static void groovy_application_class_init(GroovyApplicationClass *app_class) {
    G_APPLICATION_CLASS(app_class)->startup = groovy_application_startup;
    G_APPLICATION_CLASS(app_class)->activate = groovy_application_activate;
}

GroovyApplication *groovy_application_new(void) {
    return g_object_new(
        GROOVY_TYPE_APPLICATION,
        "application-id",
        APP_ID,
        "flags",
        G_APPLICATION_HANDLES_OPEN,
        NULL
    );
}
