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

#include "wb-media-dialog.h"

struct _WbMediaDialog
{
    GtkWindow parent_instance;
};

typedef struct
{
    GtkWidget *frame;
    GtkWidget *overlay;
} WbMediaDialogPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbMediaDialog, wb_media_dialog, GTK_TYPE_WINDOW)

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

static void
wb_media_dialog_class_init (WbMediaDialogClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/com/jonathankang/Weibird/wb-media-dialog.ui");
    gtk_widget_class_bind_template_child_private (widget_class, WbMediaDialog,
                                                  frame);
    gtk_widget_class_bind_template_child_private (widget_class, WbMediaDialog,
                                                  overlay);
    gtk_widget_class_bind_template_callback (widget_class, button_press_event_cb);
}

static void
wb_media_dialog_init (WbMediaDialog *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));
}

/**
 * wb_media_dialog_new:
 *
 * Create a new #WbMediaDialog.
 *
 * Returns: (transfer full): a newly created #WbMediaDialog
 */
WbMediaDialog *
wb_media_dialog_new (void)
{
    return g_object_new (WB_TYPE_MEDIA_DIALOG, NULL);
}
