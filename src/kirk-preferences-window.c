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

    GSettings *settings;

    GCancellable *cancellable;

    GtkWidget *qobuz_user_id_entry_row;
    GtkWidget *qobuz_token_password_entry_row;
    GtkWidget *qobuz_authorization_request_button;
    GtkWidget *qobuz_authorization_request_button_content;
    gulong qobuz_authorization_request_button_handler_id;

    GtkWidget *destination_folder_entry_row;
};

G_DEFINE_TYPE(
    KirkPreferencesWindow,
    kirk_preferences_window,
    ADW_TYPE_PREFERENCES_WINDOW
)

static void qobuz_set_verification_availability(KirkPreferencesWindow *self) {
    const gchar *user_id =
        gtk_editable_get_text(GTK_EDITABLE(self->qobuz_user_id_entry_row));
    const gchar *token = gtk_editable_get_text(  //
        GTK_EDITABLE(self->qobuz_token_password_entry_row)
    );

    gtk_widget_set_sensitive(
        GTK_WIDGET(self->qobuz_authorization_request_button),
        !(user_id[0] == '\0' || token[0] == '\0')
    );
}

static void qobuz_user_id_changed(GtkEditable *editable, gpointer user_data) {
    KirkPreferencesWindow *self = KIRK_PREFERENCES_WINDOW(user_data);

    qobuz_set_verification_availability(self);
}

static void qobuz_update_token_finish(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    kirk_secret_schema_store_password_finish(result, NULL);
}

static void qobuz_set_token(const gchar *token) {
    kirk_secret_schema_store_password(
        "Kirk: Qobuz token",
        token,
        "qobuz",
        qobuz_update_token_finish
    );
}

static void qobuz_token_changed(GtkEditable *editable, gpointer user_data) {
    KirkPreferencesWindow *self = KIRK_PREFERENCES_WINDOW(user_data);
    const gchar *token = gtk_editable_get_text(editable);

    qobuz_set_verification_availability(self);
    qobuz_set_token(token);
}

static void qobuz_send_authorization_request(
    GtkButton *button,
    gpointer user_data
);

static void qobuz_cancel_authorization_request(
    GtkButton *button,
    gpointer user_data
);

static void qobuz_reset_authorization_request_button(  //
    KirkPreferencesWindow *self
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

static void qobuz_cancel_authorization_request(
    GtkButton *button,
    gpointer user_data
) {
    KirkPreferencesWindow *self = KIRK_PREFERENCES_WINDOW(user_data);

    g_cancellable_cancel(self->cancellable);

    qobuz_reset_authorization_request_button(self);
}

static void qobuz_send_authorization_request_finish(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    KirkPreferencesWindow *self = KIRK_PREFERENCES_WINDOW(user_data);

    gchar *message = g_task_propagate_pointer(G_TASK(result), NULL);

    AdwToast *toast = adw_toast_new(message);
    adw_toast_set_timeout(toast, 2);
    adw_preferences_window_add_toast(ADW_PREFERENCES_WINDOW(self), toast);

    qobuz_reset_authorization_request_button(self);
}

static void qobuz_send_authorization_request(
    GtkButton *button,
    gpointer user_data
) {
    KirkPreferencesWindow *self = KIRK_PREFERENCES_WINDOW(user_data);

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

    self->cancellable = g_cancellable_new();
    kirk_qobuz_client_try_to_authorize(
        self->settings,
        self->cancellable,
        qobuz_send_authorization_request_finish,
        self
    );
}

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

static void select_destination_folder(GtkButton *button, gpointer user_data) {
    KirkPreferencesWindow *self = KIRK_PREFERENCES_WINDOW(user_data);

    g_autoptr(GtkFileDialog) file_dialog = gtk_file_dialog_new();
    gtk_file_dialog_select_folder(
        file_dialog,
        GTK_WINDOW(self),
        NULL,
        select_destination_folder_finish,
        self
    );
}

static void prepare_settings(KirkPreferencesWindow *self) {
    self->settings = g_settings_new(APP_ID);

    g_settings_bind(
        self->settings,
        "qobuz-user-id",
        G_OBJECT(self->qobuz_user_id_entry_row),
        "text",
        G_SETTINGS_BIND_DEFAULT
    );
    g_settings_bind(
        self->settings,
        "destination-folder-path",
        G_OBJECT(self->destination_folder_entry_row),
        "text",
        G_SETTINGS_BIND_DEFAULT
    );
}

static void prepare_qobuz_token_finish(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    KirkPreferencesWindow *self = KIRK_PREFERENCES_WINDOW(user_data);

    gchar *token = kirk_secret_schema_lookup_password_finish(result, NULL);

    if (token == NULL) {
        qobuz_set_token("");
    } else {
        gtk_editable_set_text(
            GTK_EDITABLE(self->qobuz_token_password_entry_row),
            token
        );
        g_free(token);
    }

    qobuz_set_verification_availability(self);
}

static void prepare_qobuz_token(KirkPreferencesWindow *self) {
    secret_password_lookup(
        KIRK_SECRET_SCHEMA,
        NULL,
        prepare_qobuz_token_finish,
        self,
        "schema",
        APP_ID,
        "service",
        "qobuz",
        NULL
    );
}

static void prepare_secrets(KirkPreferencesWindow *self) {
    prepare_qobuz_token(self);
}

static void prepare_handlers(KirkPreferencesWindow *self) {
    self->qobuz_authorization_request_button_handler_id = g_signal_connect(
        self->qobuz_authorization_request_button,
        "clicked",
        G_CALLBACK(qobuz_send_authorization_request),
        self
    );
}

static void kirk_preferences_window_init(KirkPreferencesWindow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));

    prepare_settings(self);
    prepare_secrets(self);
    prepare_handlers(self);
}

static void kirk_preferences_window_dispose(GObject *object) {
    KirkPreferencesWindow *self = KIRK_PREFERENCES_WINDOW(object);

    g_clear_object(&self->settings);

    g_cancellable_cancel(self->cancellable);

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
        qobuz_user_id_entry_row
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        KirkPreferencesWindow,
        qobuz_token_password_entry_row
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

    gtk_widget_class_bind_template_callback(
        widget_class,
        qobuz_user_id_changed
    );
    gtk_widget_class_bind_template_callback(widget_class, qobuz_token_changed);

    gtk_widget_class_bind_template_callback(
        widget_class,
        select_destination_folder
    );
}

KirkPreferencesWindow *kirk_preferences_window_new(
    const KirkApplication *app,
    const KirkApplicationWindow *app_win
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
