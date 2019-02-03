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
#include "wb-multi-media-widget.h"

#define MAX_WIDTH 1000
#define MAX_HEIGHT 800

enum
{
    PROP_0,
    PROP_N_CHILDS,
    PROP_PIC_URIS,
    N_PROPS
};

struct _WbMultiMediaWidget
{
    /*< private >*/
    GtkGrid parent_instance;
};

typedef struct
{
    GArray *pic_uris;
    gint n_childs;
} WbMultiMediaWidgetPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbMultiMediaWidget, wb_multi_media_widget, GTK_TYPE_GRID)

static GParamSpec *obj_properties [N_PROPS] = { NULL, };

static void
on_image_clicked (GtkButton *button,
                  gpointer user_data)
{
    gint width, height;
    GdkPixbuf *pixbuf;
    GtkWidget *image;
    GtkWidget *scrolled;
    GtkWidget *frame;
    GtkWidget *toplevel;
    WbMediaType type;
    WbMediaDialog *dialog;
    WbImageButton *image_button = WB_IMAGE_BUTTON (button);

    type = wb_image_button_get_media_type (image_button);
    pixbuf = wb_image_button_get_pixbuf (image_button);

    /* TODO: Handle clicked signal of the profile image */
    /* Return directly if it's profile image at the moment */
    if (type == WB_MEDIA_TYPE_AVATAR)
    {
        return;
    }

    /* Scale the image a bit so that it's not too large */
    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);
    if (width > MAX_WIDTH && height > MAX_HEIGHT)
    {
        gdouble scale;
        GdkPixbuf *scaled_pixbuf;

        scale = (gdouble) MAX_WIDTH / width;
        if (height * scale > MAX_HEIGHT)
        {
            scale = (gdouble) MAX_HEIGHT / height;
        }

        scaled_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
                                                 width * scale,
                                                 height * scale,
                                                 GDK_INTERP_BILINEAR);
        width *= scale;
        height *= scale;

        image = gtk_image_new_from_pixbuf (scaled_pixbuf);

        g_object_unref (scaled_pixbuf);
    }
    else
    {
        image = gtk_image_new_from_pixbuf (pixbuf);
    }

    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (scrolled),
                                                height < MAX_HEIGHT ? height : MAX_HEIGHT);
    gtk_scrolled_window_set_max_content_height (GTK_SCROLLED_WINDOW (scrolled),
                                                height > MAX_HEIGHT ? height : MAX_HEIGHT);
    gtk_scrolled_window_set_propagate_natural_width (GTK_SCROLLED_WINDOW (scrolled),
                                                     TRUE);
    gtk_container_add (GTK_CONTAINER (scrolled), image);

    dialog = wb_media_dialog_new ();
    gtk_window_set_default_size (GTK_WINDOW (dialog),
                                 width,
                                 height < MAX_HEIGHT ? height : MAX_HEIGHT);
    frame = wb_media_dialog_get_frame (dialog);
    gtk_container_add (GTK_CONTAINER (frame), scrolled);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (button));
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (toplevel));
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    gtk_widget_show_all (GTK_WIDGET (dialog));
}

static void
wb_multi_media_widget_constructed (GObject *object)
{
    gint i;
    gint left, top;
    gint width, height;
    GtkWidget *child;
    WbMultiMediaWidget *self;
    WbMultiMediaWidgetPrivate *priv;

    self = WB_MULTI_MEDIA_WIDGET (object);
    priv = wb_multi_media_widget_get_instance_private (self);

    /* Adjust the image size based on how many images there are
     * in a post. */
    if (priv->n_childs == 1)
    {
        width = height = 300;
    }
    else if (priv->n_childs == 2)
    {
        width = height = 240;
    }
    else
    {
        width = height = 150;
    }

    for (i = 0; i < priv->n_childs; i++)
    {
        child = wb_image_button_new (WB_MEDIA_TYPE_IMAGE,
                                     g_array_index (priv->pic_uris, gchar *, i),
                                     width, height);

        g_signal_connect (child, "clicked", G_CALLBACK (on_image_clicked), NULL);

        if (i < 2)
        {
            top = 0;
            left = i;
        }
        else if (i == 2)
        {
            if (priv->n_childs != 4)
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
            if (priv->n_childs != 4)
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
        gtk_grid_attach (GTK_GRID (self), child, left, top, 1, 1);
    }

    G_OBJECT_CLASS (wb_multi_media_widget_parent_class)->constructed (object);
}

static void
wb_multi_media_widget_finalize (GObject *object)
{
    G_OBJECT_CLASS (wb_multi_media_widget_parent_class)->finalize (object);
}

static void
wb_multi_media_widget_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
    WbMultiMediaWidget *self;
    WbMultiMediaWidgetPrivate *priv;

    self = WB_MULTI_MEDIA_WIDGET (object);
    priv = wb_multi_media_widget_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_N_CHILDS:
            g_value_set_int (value, priv->n_childs);
            break;
        case PROP_PIC_URIS:
            g_value_set_pointer (value, priv->pic_uris);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
wb_multi_media_widget_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
    WbMultiMediaWidget *self;
    WbMultiMediaWidgetPrivate *priv;

    self = WB_MULTI_MEDIA_WIDGET (object);
    priv = wb_multi_media_widget_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_N_CHILDS:
            priv->n_childs = g_value_get_int (value);
            break;
        case PROP_PIC_URIS:
            priv->pic_uris = g_value_get_pointer (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
wb_multi_media_widget_class_init(WbMultiMediaWidgetClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wb_multi_media_widget_constructed;
    object_class->finalize = wb_multi_media_widget_finalize;
    object_class->get_property = wb_multi_media_widget_get_property;
    object_class->set_property = wb_multi_media_widget_set_property;

    obj_properties[PROP_N_CHILDS] = g_param_spec_int ("n-childs",
                                                      "Child Number",
                                                      "Number of child widgets",
                                                      1, 9, 1,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT_ONLY);
    obj_properties[PROP_PIC_URIS] = g_param_spec_pointer ("pic-uris",
                                                          "Picture URIs",
                                                          "URI for pictures",
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS);
    g_object_class_install_properties (object_class, N_PROPS, obj_properties);
}

static void
wb_multi_media_widget_init (WbMultiMediaWidget *self)
{
}

/**
 * wb_multi_media_widget_new:
 *
 * Create a new #WbMultiMediaWidget.
 *
 * Returns: (transfer full): a newly created #WbMultiMediaWidget
 */
GtkWidget *
wb_multi_media_widget_new (gint n_childs, const GArray *pic_uris)
{
    return g_object_new (WB_TYPE_MULTI_MEDIA_WIDGET,
                         "n-childs", n_childs,
                         "pic-uris", pic_uris,
                         NULL);
}
