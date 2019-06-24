/*
 *  Weibird - View and compose weibo
 *  Copyright (C) 2018-2019 Jonathan Kang <jonathankang@gnome.org>.
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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

#include "wb-media-dialog.h"
#include "wb-util.h"

struct _WbMediaDialog
{
    GtkWindow parent_instance;
};

typedef struct
{
    const GArray *pic_uris;
    gint nth_media;
    GtkWidget *cur_image;
    GtkWidget *frame;
    GtkWidget *scrolled;
    GtkWidget *previous_revealer;
    GtkWidget *next_revealer;
} WbMediaDialogPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbMediaDialog, wb_media_dialog, GTK_TYPE_WINDOW)

static void change_media (WbMediaDialog *media_dialog,
                          gboolean previous);
static void on_message_complete (SoupSession *session,
                                 SoupMessage *msg,
                                 gpointer user_data);

GtkWidget *
wb_media_dialog_get_frame (WbMediaDialog *self)
{
    WbMediaDialogPrivate *priv = wb_media_dialog_get_instance_private (self);

    return priv->frame;
}

static gboolean
button_press_event_cb (GtkWidget *widget,
                       GdkEventButton *event)
{
    gtk_widget_destroy (widget);

    return GDK_EVENT_PROPAGATE;
}

static gboolean
key_press_event_cb (GtkWidget *widget,
                    GdkEvent *event,
                    gpointer user_data)
{
    WbMediaDialog *media_dialog = WB_MEDIA_DIALOG (user_data);

    if (event->key.keyval == GDK_KEY_Left)
    {
        change_media (media_dialog, TRUE);
    }
    else if (event->key.keyval == GDK_KEY_Right)
    {
        change_media (media_dialog, FALSE);
    }
    else
    {
        gtk_widget_destroy (widget);
    }

    return GDK_EVENT_PROPAGATE;
}

static gboolean
enter_notify_event_cb (GtkWidget *widget,
                       GdkEvent *event,
                       gpointer user_data)
{
    WbMediaDialog *self = WB_MEDIA_DIALOG (user_data);
    WbMediaDialogPrivate *priv = wb_media_dialog_get_instance_private (self);

    gtk_revealer_set_reveal_child (GTK_REVEALER (priv->previous_revealer), TRUE);
    gtk_revealer_set_reveal_child (GTK_REVEALER (priv->next_revealer), TRUE);

    return GDK_EVENT_PROPAGATE;
}

static gboolean
leave_notify_event_cb (GtkWidget *widget,
                       GdkEvent *event,
                       gpointer user_data)
{
    WbMediaDialog *self = WB_MEDIA_DIALOG (user_data);
    WbMediaDialogPrivate *priv = wb_media_dialog_get_instance_private (self);

    gtk_revealer_set_reveal_child (GTK_REVEALER (priv->previous_revealer), FALSE);
    gtk_revealer_set_reveal_child (GTK_REVEALER (priv->next_revealer), FALSE);

    return GDK_EVENT_PROPAGATE;
}

static void
wb_media_dialog_download_original_image (WbMediaDialog *self,
                                         const gchar *thumbnail_uri)
{
    gchar *original_uri;
    SoupMessage *msg;

    original_uri = wb_util_thumbnail_to_original (thumbnail_uri);
    msg = soup_message_new (SOUP_METHOD_GET, original_uri);
    soup_session_queue_message (SOUPSESSION, msg,
                                on_message_complete, self);

    g_free (original_uri);
}

static void
change_media (WbMediaDialog *media_dialog,
              gboolean previous)
{
    const gchar *thumbnail_uri;
    WbMediaDialogPrivate *priv;

    priv = wb_media_dialog_get_instance_private (media_dialog);

    if (previous && priv->nth_media == 1)
    {
        return;
    }
    else if (!previous && priv->nth_media == priv->pic_uris->len)
    {
        return;
    }

    gtk_container_remove (GTK_CONTAINER (priv->scrolled), priv->cur_image);

    priv->nth_media = previous ? priv->nth_media - 1 : priv->nth_media + 1;

    /* Reveal the previous and next button or not */
    gtk_widget_set_visible (priv->previous_revealer, priv->nth_media != 1);
    gtk_widget_set_visible (priv->next_revealer,
                            priv->nth_media != priv->pic_uris->len);

    thumbnail_uri = g_array_index (priv->pic_uris, gchar *, priv->nth_media - 1);
    wb_media_dialog_download_original_image (media_dialog, thumbnail_uri);
}

static void
previous_button_clicked_cb (GtkButton *button,
                            gpointer user_data)
{
    WbMediaDialog *media_dialog;

    media_dialog = WB_MEDIA_DIALOG (user_data);

    change_media (media_dialog, TRUE);
}

static void
next_button_clicked_cb (GtkButton *button,
                        gpointer user_data)
{
    WbMediaDialog *media_dialog;

    media_dialog = WB_MEDIA_DIALOG (user_data);

    change_media (media_dialog, FALSE);
}

static void
on_message_complete (SoupSession *session,
                     SoupMessage *msg,
                     gpointer user_data)
{
    g_autoptr(GInputStream) stream = NULL;
    gint width;
    gint height;
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    WbMediaDialog *self = WB_MEDIA_DIALOG (user_data);
    WbMediaDialogPrivate *priv = wb_media_dialog_get_instance_private (self);

    if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
        if (msg->status_code != SOUP_STATUS_CANCELLED)
        {
            g_warning ("Failed to get image: %d %s.\n",
                       msg->status_code, msg->reason_phrase);
        }
        return;
    }

    stream = g_memory_input_stream_new_from_data (msg->response_body->data,
                                                  msg->response_body->length,
                                                  NULL);
    pixbuf = gdk_pixbuf_new_from_stream (stream, NULL, &error);
    if (error != NULL)
    {
        g_warning ("Unable to create pixbuf: %s",
                   error->message);
        g_clear_error (&error);
    }

    /* Scale the image a bit so that it's not too large */
    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);
    priv->cur_image = wb_util_scale_image (pixbuf, &width, &height);

    gtk_widget_show (priv->cur_image);
    gtk_container_add (GTK_CONTAINER (priv->scrolled), priv->cur_image);
    gtk_window_resize (GTK_WINDOW (self),
                       width,
                       height < MAX_HEIGHT ? height : MAX_HEIGHT);

    g_object_unref (pixbuf);
}

static void
wb_media_dialog_setup (WbMediaDialog *self)
{
    const gchar *thumbnail_uri;
    WbMediaDialogPrivate *priv;

    priv = wb_media_dialog_get_instance_private (self);

    thumbnail_uri = g_array_index (priv->pic_uris, gchar *, priv->nth_media - 1);
    wb_media_dialog_download_original_image (self, thumbnail_uri);
}

static void
wb_media_dialog_class_init (WbMediaDialogClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/com/jonathankang/Weibird/wb-media-dialog.ui");
    gtk_widget_class_bind_template_child_private (widget_class, WbMediaDialog,
                                                  frame);
    gtk_widget_class_bind_template_child_private (widget_class, WbMediaDialog,
                                                  scrolled);
    gtk_widget_class_bind_template_child_private (widget_class, WbMediaDialog,
                                                  previous_revealer);
    gtk_widget_class_bind_template_child_private (widget_class, WbMediaDialog,
                                                  next_revealer);
    gtk_widget_class_bind_template_callback (widget_class, button_press_event_cb);
    gtk_widget_class_bind_template_callback (widget_class, key_press_event_cb);
    gtk_widget_class_bind_template_callback (widget_class, enter_notify_event_cb);
    gtk_widget_class_bind_template_callback (widget_class, leave_notify_event_cb);
    gtk_widget_class_bind_template_callback (widget_class, previous_button_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, next_button_clicked_cb);
}

static void
wb_media_dialog_init (WbMediaDialog *self)
{
    WbMediaDialogPrivate *priv = wb_media_dialog_get_instance_private (self);

    gtk_widget_init_template (GTK_WIDGET (self));

    gtk_revealer_set_transition_type (GTK_REVEALER (priv->previous_revealer),
                                      GTK_REVEALER_TRANSITION_TYPE_CROSSFADE);
    gtk_revealer_set_transition_type (GTK_REVEALER (priv->next_revealer),
                                      GTK_REVEALER_TRANSITION_TYPE_CROSSFADE);
    gtk_revealer_set_transition_duration (GTK_REVEALER (priv->previous_revealer),
                                          1000);
    gtk_revealer_set_transition_duration (GTK_REVEALER (priv->next_revealer),
                                          1000);
}

/**
 * wb_media_dialog_new:
 *
 * Create a new #WbMediaDialog.
 *
 * Returns: (transfer full): a newly created #WbMediaDialog
 */
WbMediaDialog *
wb_media_dialog_new (const GArray *pic_uris,
                     gint nth_media)
{
    WbMediaDialog *self;
    WbMediaDialogPrivate *priv;

    self = g_object_new (WB_TYPE_MEDIA_DIALOG, NULL);
    priv = wb_media_dialog_get_instance_private (self);

    priv->pic_uris = pic_uris;
    priv->nth_media = nth_media;

    /* Whether to reveal the previous and next button or not */
    if (priv->nth_media == 1)
    {
        gtk_widget_hide (priv->previous_revealer);
    }
    if (priv->nth_media == priv->pic_uris->len)
    {
        gtk_widget_hide (priv->next_revealer);
    }

    wb_media_dialog_setup (self);

    return self;
}
