/*
 *  Weibird - view and compose weibo
 *  copyright (c) 2018-2019 jonathan kang <jonathankang@gnome.org>.
 *
 *  this program is free software: you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation, either version 3 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program.  if not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#include "wb-image-button.h"
#include "wb-media-dialog.h"
#include "wb-util.h"

struct _WbMediaDialog
{
    GtkWindow parent_instance;
};

typedef struct
{
    GArray *images;
    gint nth_media;
    GtkWidget *cur_image;
    GtkWidget *frame;
    GtkWidget *previous_revealer;
    GtkWidget *next_revealer;
} WbMediaDialogPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbMediaDialog, wb_media_dialog, GTK_TYPE_WINDOW)

static void change_media (WbMediaDialog *media_dialog,
                          gboolean previous);

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
change_media (WbMediaDialog *media_dialog,
              gboolean previous)
{
    gint width;
    gint height;
    GdkPixbuf *pixbuf;
    GtkWidget *image;
    GtkWidget *scrolled;
    WbImageButton *cur_image;
    WbMediaDialogPrivate *priv;

    priv = wb_media_dialog_get_instance_private (media_dialog);

    if (previous && priv->nth_media == 1)
    {
        return;
    }
    else if (!previous && priv->nth_media == priv->images->len)
    {
        return;
    }

    gtk_widget_destroy (priv->cur_image);

    priv->nth_media = previous ? priv->nth_media - 1 : priv->nth_media + 1;

    cur_image = g_array_index (priv->images, WbImageButton *, priv->nth_media - 1);
    pixbuf = wb_image_button_get_pixbuf (cur_image);

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);
    image = wb_util_scale_image (pixbuf, &width, &height);

    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (scrolled),
                                                height < MAX_HEIGHT ? height : MAX_HEIGHT);
    gtk_scrolled_window_set_max_content_height (GTK_SCROLLED_WINDOW (scrolled),
                                                height > MAX_HEIGHT ? height : MAX_HEIGHT);
    gtk_scrolled_window_set_propagate_natural_width (GTK_SCROLLED_WINDOW (scrolled),
                                                     TRUE);
    gtk_container_add (GTK_CONTAINER (scrolled), image);
    priv->cur_image = scrolled;

    gtk_widget_set_size_request (GTK_WIDGET (media_dialog),
                                 width,
                                 height < MAX_HEIGHT ? height : MAX_HEIGHT);
    gtk_container_add (GTK_CONTAINER (priv->frame), scrolled);

    gtk_widget_show_all (GTK_WIDGET (media_dialog));
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
wb_media_dialog_class_init (WbMediaDialogClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/com/jonathankang/Weibird/wb-media-dialog.ui");
    gtk_widget_class_bind_template_child_private (widget_class, WbMediaDialog,
                                                  frame);
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
wb_media_dialog_new (GArray *images,
                     gint nth_media,
                     GtkWidget *cur_image)
{
    WbMediaDialog *self;
    WbMediaDialogPrivate *priv;

    self = g_object_new (WB_TYPE_MEDIA_DIALOG, NULL);
    priv = wb_media_dialog_get_instance_private (self);

    priv->images = images;
    priv->nth_media = nth_media;
    priv->cur_image = cur_image;

    return self;
}
