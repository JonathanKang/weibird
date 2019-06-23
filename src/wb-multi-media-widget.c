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

#include <gtk/gtk.h>

#include "wb-image-button.h"
#include "wb-media-dialog.h"
#include "wb-multi-media-widget.h"
#include "wb-util.h"

struct _WbMultiMediaWidget
{
    /*< private >*/
    GtkGrid parent_instance;
};

typedef struct
{
    const GArray *pic_uris;
} WbMultiMediaWidgetPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbMultiMediaWidget, wb_multi_media_widget, GTK_TYPE_GRID)

static void
on_image_clicked (GtkButton *button,
                  gpointer user_data)
{
    gint nth_media;
    GtkWidget *toplevel;
    WbMediaDialog *dialog;
    WbImageButton *image_button;
    WbMultiMediaWidget *mm_widget;
    WbMultiMediaWidgetPrivate *priv;

    image_button = WB_IMAGE_BUTTON (button);
    mm_widget = WB_MULTI_MEDIA_WIDGET (user_data);
    priv = wb_multi_media_widget_get_instance_private (mm_widget);

    nth_media = wb_image_button_get_nth_media (image_button);
    dialog = wb_media_dialog_new (priv->pic_uris, nth_media);
    /* FIXME: Initialize the dialog in a proper size. */
    gtk_window_set_default_size (GTK_WINDOW (dialog), 100, 100);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (button));
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (toplevel));

    gtk_widget_show_all (GTK_WIDGET (dialog));
}

void
wb_multi_media_widget_populate_images (WbMultiMediaWidget *self,
                                       const GArray *pic_uris)
{
    gint i;
    gint n_childs;
    gint left, top;
    gint width, height;
    WbImageButton *button;
    WbMultiMediaWidgetPrivate *priv;

    priv = wb_multi_media_widget_get_instance_private (self);
    priv->pic_uris = pic_uris;

    n_childs = pic_uris->len;

    /* Adjust the image size based on how many images there are
     * in a post. */
    if (n_childs == 1)
    {
        width = height = 300;
    }
    else if (n_childs == 2 || n_childs == 4)
    {
        width = height = 240;
    }
    else
    {
        width = height = 150;
    }

    for (i = 0; i < n_childs; i++)
    {
        button = wb_image_button_new (WB_MEDIA_TYPE_IMAGE,
                                      g_array_index (pic_uris, gchar *, i),
                                      i + 1, width, height);

        g_signal_connect (button, "clicked",
                          G_CALLBACK (on_image_clicked), self);

        if (i < 2)
        {
            top = 0;
            left = i;
        }
        else if (i == 2)
        {
            if (n_childs != 4)
            {
                top = 0;
                left = i;
            }
            else
            {
                top = 1;
                left = 0;
            }
        }
        else if (i >= 3 && i < 6)
        {
            if (n_childs != 4)
            {
                top = 1;
                left = i - 3;
            }
            else
            {
                top = 1;
                left = i - 2;
            }
        }
        else
        {
            top = 2;
            left = i - 6;
        }
        gtk_grid_attach (GTK_GRID (self), GTK_WIDGET (button),
                         left, top, 1, 1);
    }
}

static void
wb_multi_media_widget_class_init(WbMultiMediaWidgetClass *klass)
{
}

static void
wb_multi_media_widget_init (WbMultiMediaWidget *self)
{
    gtk_grid_set_column_spacing (GTK_GRID (self), 3);
    gtk_grid_set_row_spacing (GTK_GRID (self), 3);
}

/**
 * wb_multi_media_widget_new:
 *
 * Create a new #WbMultiMediaWidget.
 *
 * Returns: (transfer full): a newly created #WbMultiMediaWidget
 */
WbMultiMediaWidget *
wb_multi_media_widget_new (void)
{
    return g_object_new (WB_TYPE_MULTI_MEDIA_WIDGET, NULL);
}
