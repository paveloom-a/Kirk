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

#include <adwaita.h>

struct _GroovyApplication {
    AdwApplication parent;
};

G_DEFINE_TYPE(GroovyApplication, groovy_application, ADW_TYPE_APPLICATION)

static void groovy_application_init(GroovyApplication *self) {}

static void quit_activated(
    GSimpleAction *action,
    GVariant *parameter,
    gpointer user_data
) {
    g_application_quit(G_APPLICATION(user_data));
}

static void groovy_application_startup(GApplication *g_app) {
    GroovyApplication *self = GROOVY_APPLICATION(g_app);

    G_APPLICATION_CLASS(groovy_application_parent_class)->startup(g_app);

    const GActionEntry entries[] = {
        {.name = "quit", .activate = quit_activated},
    };
    g_action_map_add_action_entries(
        G_ACTION_MAP(self),
        entries,
        G_N_ELEMENTS(entries),
        self
    );

    const char *quit_accels[] = {"<primary>q", NULL};
    gtk_application_set_accels_for_action(
        GTK_APPLICATION(self),
        "app.quit",
        quit_accels
    );
}

static void groovy_application_activate(GApplication *g_app) {
    GroovyApplication *self = GROOVY_APPLICATION(g_app);

    GroovyApplicationWindow *win = groovy_application_window_new(self);
    gtk_window_present(GTK_WINDOW(win));
}

static void groovy_application_class_init(GroovyApplicationClass *klass) {
    G_APPLICATION_CLASS(klass)->startup = groovy_application_startup;
    G_APPLICATION_CLASS(klass)->activate = groovy_application_activate;
}

GroovyApplication *groovy_application_new() {
    return g_object_new(
        GROOVY_TYPE_APPLICATION,
        "application-id",
        APP_ID,
        "flags",
        G_APPLICATION_HANDLES_OPEN,
        NULL
    );
}
