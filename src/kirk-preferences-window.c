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
#include "src/kirk-qobuz-client.h"
#include "src/kirk-secret-schema.h"

#include <adwaita.h>
#include <libsecret/secret.h>

struct _KirkPreferencesWindow {
    AdwPreferencesWindow parent;

    GSettings* settings;

    GCancellable* cancellable;

    GtkWidget* qobuz_token_password_entry_row;
    GtkWidget* qobuz_app_id_password_entry_row;
    GtkWidget* qobuz_fetch_app_id_stack;
    GtkWidget* qobuz_fetch_app_id_button;
    GtkWidget* qobuz_fetch_app_id_spinner;
    GtkWidget* qobuz_authorization_request_button;
    GtkWidget* qobuz_authorization_request_button_content;
    gulong qobuz_authorization_request_button_handler_id;

    GtkWidget* destination_folder_entry_row;
};

G_DEFINE_TYPE(
    KirkPreferencesWindow,
    kirk_preferences_window,
    ADW_TYPE_PREFERENCES_WINDOW
)

static void qobuz_set_authorization_availability(KirkPreferencesWindow* self) {
    const gchar* token = gtk_editable_get_text(  //
        GTK_EDITABLE(self->qobuz_token_password_entry_row)
    );

    const gchar* app_id = gtk_editable_get_text(  //
        GTK_EDITABLE(self->qobuz_app_id_password_entry_row)
    );

    gtk_widget_set_sensitive(
        GTK_WIDGET(self->qobuz_authorization_request_button),
        (token && token[0]) && (app_id && app_id[0])
    );
}

static void qobuz_send_authorization_request(
    GtkButton* button,
    gpointer user_data
);

static void qobuz_cancel_authorization_request(
    GtkButton* button,
    gpointer user_data
);

static void qobuz_set_authorization_request_button_to_send(
    KirkPreferencesWindow* self
) {
    adw_button_content_set_label(
        ADW_BUTTON_CONTENT(self->qobuz_authorization_request_button_content),
        "Send authorization request"
    );

    g_signal_handler_disconnect(
        self->qobuz_authorization_request_button,
        self->qobuz_authorization_request_button_handler_id
    );
    self->qobuz_authorization_request_button_handler_id = g_signal_connect(
        self->qobuz_authorization_request_button,
        "clicked",
        G_CALLBACK(qobuz_send_authorization_request),
        self
    );
}

static void qobuz_set_authorization_request_button_to_cancel(
    KirkPreferencesWindow* self
) {
    adw_button_content_set_label(
        ADW_BUTTON_CONTENT(self->qobuz_authorization_request_button_content),
        "Cancel authorization request"
    );

    g_signal_handler_disconnect(
        self->qobuz_authorization_request_button,
        self->qobuz_authorization_request_button_handler_id
    );
    self->qobuz_authorization_request_button_handler_id = g_signal_connect(
        self->qobuz_authorization_request_button,
        "clicked",
        G_CALLBACK(qobuz_cancel_authorization_request),
        self
    );
}

static void qobuz_cancel_authorization_request(
    GtkButton* button,
    gpointer user_data
) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(user_data);

    g_cancellable_cancel(self->cancellable);

    qobuz_set_authorization_request_button_to_send(self);
}

static void qobuz_send_authorization_request_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(source_object);

    const gchar* message = g_task_propagate_pointer(G_TASK(result), NULL);

    AdwToast* toast = adw_toast_new(message);
    adw_toast_set_timeout(toast, 2);
    adw_preferences_window_add_toast(ADW_PREFERENCES_WINDOW(self), toast);

    qobuz_set_authorization_request_button_to_send(self);
}

static void qobuz_send_authorization_request(
    GtkButton* button,
    gpointer user_data
) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(user_data);

    qobuz_set_authorization_request_button_to_cancel(self);

    self->cancellable = g_cancellable_new();
    kirk_qobuz_client_try_to_authorize(
        G_OBJECT(self),
        self->cancellable,
        qobuz_send_authorization_request_finish,
        NULL
    );
}

static void prepare_settings(KirkPreferencesWindow* self) {
    self->settings = g_settings_new(APP_ID);

    g_settings_bind(
        self->settings,
        "destination-folder-path",
        G_OBJECT(self->destination_folder_entry_row),
        "text",
        G_SETTINGS_BIND_DEFAULT
    );
}

static void qobuz_lookup_token_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(user_data);

    gchar* token = kirk_secret_schema_lookup_password_finish(result, NULL);

    if (token) {
        gtk_editable_set_text(
            GTK_EDITABLE(self->qobuz_token_password_entry_row),
            token
        );
        g_free(token);
    } else {
        kirk_secret_schema_store_qobuz_token("");
    }

    qobuz_set_authorization_availability(self);
}

static void qobuz_lookup_token(KirkPreferencesWindow* self) {
    kirk_secret_schema_lookup_password(
        "qobuz",
        "token",
        NULL,
        qobuz_lookup_token_finish,
        self
    );
}

static void qobuz_lookup_app_id_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(user_data);

    gchar* app_id = kirk_secret_schema_lookup_password_finish(result, NULL);

    if (app_id) {
        gtk_editable_set_text(
            GTK_EDITABLE(self->qobuz_app_id_password_entry_row),
            app_id
        );
        g_free(app_id);
    } else {
        kirk_secret_schema_store_qobuz_app_id("");
    }

    qobuz_set_authorization_availability(self);
}

static void qobuz_lookup_app_id(KirkPreferencesWindow* self) {
    kirk_secret_schema_lookup_password(
        "qobuz",
        "app_id",
        NULL,
        qobuz_lookup_app_id_finish,
        self
    );
}

static void prepare_secrets(KirkPreferencesWindow* self) {
    qobuz_lookup_token(self);
    qobuz_lookup_app_id(self);
}

static void prepare_handlers(KirkPreferencesWindow* self) {
    self->qobuz_authorization_request_button_handler_id = g_signal_connect(
        self->qobuz_authorization_request_button,
        "clicked",
        G_CALLBACK(qobuz_send_authorization_request),
        self
    );
}

static void kirk_preferences_window_init(KirkPreferencesWindow* self) {
    gtk_widget_init_template(GTK_WIDGET(self));

    prepare_settings(self);
    prepare_secrets(self);
    prepare_handlers(self);
}

static void kirk_preferences_window_dispose(GObject* object) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(object);

    g_cancellable_cancel(self->cancellable);

    g_clear_object(&self->settings);

    gtk_widget_dispose_template(GTK_WIDGET(self), KIRK_TYPE_PREFERENCES_WINDOW);

    G_OBJECT_CLASS(kirk_preferences_window_parent_class)->dispose(object);
}

static void qobuz_token_changed(GtkEditable* editable, gpointer user_data) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(user_data);
    const gchar* token = gtk_editable_get_text(editable);

    qobuz_set_authorization_availability(self);
    kirk_secret_schema_store_qobuz_token(token);
}

static void qobuz_app_id_changed(GtkEditable* editable, gpointer user_data) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(user_data);
    const gchar* app_id = gtk_editable_get_text(editable);

    qobuz_set_authorization_availability(self);
    kirk_secret_schema_store_qobuz_app_id(app_id);
}

static void qobuz_fetch_app_id_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(source_object);

    if (!gtk_widget_is_visible(GTK_WIDGET(self))) {
        return;
    }

    gtk_stack_set_visible_child(
        GTK_STACK(self->qobuz_fetch_app_id_stack),
        self->qobuz_fetch_app_id_button
    );
    if (gtk_widget_has_focus(self->qobuz_fetch_app_id_spinner)) {
        gtk_widget_grab_focus(self->qobuz_fetch_app_id_button);
    }

    const gchar* message = g_task_propagate_pointer(G_TASK(result), NULL);

    AdwToast* toast = adw_toast_new(message);
    adw_toast_set_timeout(toast, 2);
    adw_preferences_window_add_toast(ADW_PREFERENCES_WINDOW(self), toast);

    qobuz_lookup_app_id(self);
}

static void qobuz_fetch_app_id(GtkButton* button, gpointer user_data) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(user_data);

    gtk_stack_set_visible_child(
        GTK_STACK(self->qobuz_fetch_app_id_stack),
        self->qobuz_fetch_app_id_spinner
    );
    gtk_widget_grab_focus(self->qobuz_fetch_app_id_spinner);

    self->cancellable = g_cancellable_new();
    kirk_qobuz_client_try_to_fetch_app_id(
        G_OBJECT(self),
        self->cancellable,
        qobuz_fetch_app_id_finish,
        NULL
    );
}

static void select_destination_folder_finish(
    GObject* source_object,
    GAsyncResult* result,
    gpointer user_data
) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(user_data);
    GtkFileDialog* file_dialog = GTK_FILE_DIALOG(source_object);

    g_autoptr(GFile) file =
        gtk_file_dialog_select_folder_finish(file_dialog, result, NULL);

    if (!file) {
        return;
    }

    const gchar* destination_folder_path = g_file_peek_path(file);
    g_settings_set_string(
        self->settings,
        "destination-folder-path",
        destination_folder_path
    );
}

static void select_destination_folder(GtkButton* button, gpointer user_data) {
    KirkPreferencesWindow* self = KIRK_PREFERENCES_WINDOW(user_data);

    g_autoptr(GtkFileDialog) file_dialog = gtk_file_dialog_new();
    gtk_file_dialog_select_folder(
        file_dialog,
        GTK_WINDOW(self),
        NULL,
        select_destination_folder_finish,
        self
    );
}

static void kirk_preferences_window_class_init(KirkPreferencesWindowClass* klass
) {
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(
        widget_class,
        APP_RESOURCES_PATH "gtk/kirk-preferences-window.ui"
    );

    object_class->dispose = kirk_preferences_window_dispose;

    gtk_widget_class_bind_template_child(
        widget_class,
        KirkPreferencesWindow,
        qobuz_token_password_entry_row
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        KirkPreferencesWindow,
        qobuz_app_id_password_entry_row
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        KirkPreferencesWindow,
        qobuz_fetch_app_id_stack
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        KirkPreferencesWindow,
        qobuz_fetch_app_id_button
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        KirkPreferencesWindow,
        qobuz_fetch_app_id_spinner
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        KirkPreferencesWindow,
        qobuz_authorization_request_button
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        KirkPreferencesWindow,
        qobuz_authorization_request_button_content
    );

    gtk_widget_class_bind_template_child(
        widget_class,
        KirkPreferencesWindow,
        destination_folder_entry_row
    );

    gtk_widget_class_bind_template_callback(widget_class, qobuz_token_changed);
    gtk_widget_class_bind_template_callback(widget_class, qobuz_app_id_changed);
    gtk_widget_class_bind_template_callback(widget_class, qobuz_fetch_app_id);

    gtk_widget_class_bind_template_callback(
        widget_class,
        select_destination_folder
    );
}

KirkPreferencesWindow* kirk_preferences_window_new(
    const KirkApplication* app,
    const KirkApplicationWindow* app_win
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
