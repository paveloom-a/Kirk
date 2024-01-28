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

#include "kirk-add-release-window-search-item.h"

#include "src/kirk-add-release-window-search-item.h"

#include <adwaita.h>

struct _KirkAddReleaseWindowSearchItem {
    GObject parent;

    gchar* artist;
    gchar* title;
};

G_DEFINE_TYPE(
    KirkAddReleaseWindowSearchItem,
    kirk_add_release_window_search_item,
    G_TYPE_OBJECT
)

static void kirk_add_release_window_search_item_init(
    KirkAddReleaseWindowSearchItem* self
) {}

static void kirk_add_release_window_search_item_dispose(GObject* object) {
    KirkAddReleaseWindowSearchItem* self =
        KIRK_ADD_RELEASE_WINDOW_SEARCH_ITEM(object);

    g_free(self->artist);
    g_free(self->title);

    G_OBJECT_CLASS(kirk_add_release_window_search_item_parent_class)
        ->dispose(object);
}

typedef enum {
    PROP_ARTIST = 1,
    PROP_TITLE,
    N_PROPERTIES
} KirkAddReleaseWindowSearchItemProperty;

static GParamSpec* obj_properties[N_PROPERTIES] = {
    NULL,
};

static void kirk_add_release_window_search_item_set_property(
    GObject* object,
    guint property_id,
    const GValue* value,
    GParamSpec* pspec
) {
    KirkAddReleaseWindowSearchItem* self =
        KIRK_ADD_RELEASE_WINDOW_SEARCH_ITEM(object);

    switch ((KirkAddReleaseWindowSearchItemProperty)property_id) {
    case PROP_ARTIST:
        g_free(self->artist);
        self->artist = g_value_dup_string(value);
        break;
    case PROP_TITLE:
        self->title = g_value_dup_string(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void kirk_add_release_window_search_item_get_property(
    GObject* object,
    guint property_id,
    GValue* value,
    GParamSpec* pspec
) {
    KirkAddReleaseWindowSearchItem* self =
        KIRK_ADD_RELEASE_WINDOW_SEARCH_ITEM(object);

    switch ((KirkAddReleaseWindowSearchItemProperty)property_id) {
    case PROP_ARTIST:
        g_value_set_string(value, self->artist);
        break;
    case PROP_TITLE:
        g_value_set_string(value, self->title);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void kirk_add_release_window_search_item_class_init(
    KirkAddReleaseWindowSearchItemClass* klass
) {
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = kirk_add_release_window_search_item_dispose;
    object_class->get_property =
        kirk_add_release_window_search_item_get_property;
    object_class->set_property =
        kirk_add_release_window_search_item_set_property;

    obj_properties[PROP_ARTIST] = g_param_spec_string(
        "artist",
        "Artist",
        "Name of the artist of the release.",
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE
    );

    obj_properties[PROP_TITLE] = g_param_spec_string(
        "title",
        "Title",
        "Title of the release.",
        NULL,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE
    );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        obj_properties
    );
}

KirkAddReleaseWindowSearchItem* kirk_add_release_window_search_item_new(
    gchar* artist,
    gchar* title
) {
    return g_object_new(
        KIRK_TYPE_ADD_RELEASE_WINDOW_SEARCH_ITEM,
        "artist",
        artist,
        "title",
        title,
        NULL
    );
}
