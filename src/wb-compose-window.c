/*
 *  Weibird - View and compose weibo
 *  Copyright (C) 2019 Jonathan Kang <jonathankang@gnome.org>.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#include "wb-compose-window.h"

struct _WbComposeWindow
{
		GtkDialog parent_instance;
};

typedef struct
{
    GtkWidget *compose_entry;
    GtkWidget *send_button;
} WbComposeWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbComposeWindow, wb_compose_window, GTK_TYPE_DIALOG)

GtkWidget *
wb_compose_window_get_compose_entry (WbComposeWindow *self)
{
    WbComposeWindowPrivate *priv;

    g_return_val_if_fail (WB_IS_COMPOSE_WINDOW (self), NULL);

    priv = wb_compose_window_get_instance_private (self);

    return priv->compose_entry;
}

static void
compose_entry_changed_cb (GtkEntry *entry,
                          GParamSpec *pspec,
                          gpointer user_data)
{
    const gchar *comment;
    WbComposeWindow *self;
    WbComposeWindowPrivate *priv;

    self = WB_COMPOSE_WINDOW (user_data);
    priv = wb_compose_window_get_instance_private (self);

    comment = gtk_entry_get_text (GTK_ENTRY (priv->compose_entry));
    gtk_widget_set_sensitive (priv->send_button,
                              g_strcmp0 (comment, "") == 0 ? FALSE : TRUE);
}

static void
wb_compose_window_finalize (GObject *object)
{
    G_OBJECT_CLASS (wb_compose_window_parent_class)->finalize (object);
}

static void
wb_compose_window_class_init (WbComposeWindowClass *klass)
{
		GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

		object_class->finalize = wb_compose_window_finalize;

    gtk_widget_class_set_template_from_resource (widget_class, "/com/jonathankang/Weibird/wb-compose-window.ui");
    gtk_widget_class_bind_template_child_private (widget_class, WbComposeWindow,
                                                  compose_entry);
    gtk_widget_class_bind_template_child_private (widget_class, WbComposeWindow,
                                                  send_button);
}

static void
wb_compose_window_init (WbComposeWindow *self)
{
    WbComposeWindowPrivate *priv;

    priv = wb_compose_window_get_instance_private (self);

    gtk_widget_init_template (GTK_WIDGET (self));

    g_signal_connect (priv->compose_entry, "notify::text",
                      G_CALLBACK (compose_entry_changed_cb), self);
}

/**
 * wb_compose_window_new:
 *
 * Create a new #WbComposeWindow.
 *
 * Returns: (transfer full): a newly created #WbComposeWindow
 */
WbComposeWindow *
wb_compose_window_new (GtkWindow *parent)
{
    return g_object_new (WB_TYPE_COMPOSE_WINDOW,
                         "modal", TRUE,
                         "transient-for", parent,
                         "use-header-bar", 1,
                         NULL);
}
