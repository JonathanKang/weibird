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
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <libsoup/soup.h>

#include "wb-enums.h"
#include "wb-image-button.h"

enum
{
    PROP_0,
    PROP_URI,
    PROP_MEDIA_TYPE,
    PROP_NTH_MEDIA,
    PROP_WIDTH,
    PROP_HEIGHT,
    N_PROPS
};

struct _WbImageButton
{
    /*< private >*/
    GtkButton parent_instance;
};

typedef struct
{
    gchar *uri;
    gint nth_media;
    gint width;
    gint height;
    GdkPixbuf *pixbuf;
    GtkWidget *image;
    SoupSession *session;
    WbMediaType type;
} WbImageButtonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbImageButton, wb_image_button, GTK_TYPE_BUTTON)

static GParamSpec *obj_properties [N_PROPS] = { NULL, };

gint
wb_image_button_get_nth_media (WbImageButton *self)
{
    WbImageButtonPrivate *priv = wb_image_button_get_instance_private (self);

    return priv->nth_media;
}

WbMediaType
wb_image_button_get_media_type (WbImageButton *self)
{
    WbImageButtonPrivate *priv = wb_image_button_get_instance_private (self);

    return priv->type;
}

GdkPixbuf *
wb_image_button_get_pixbuf (WbImageButton *self)
{
    WbImageButtonPrivate *priv = wb_image_button_get_instance_private (self);

    return priv->pixbuf;
}

static void
on_message_complete (SoupSession *session,
                     SoupMessage *msg,
                     gpointer user_data)
{
    g_autoptr(GInputStream) stream = NULL;
    GError *error = NULL;
    WbImageButton *self = WB_IMAGE_BUTTON (user_data);
    WbImageButtonPrivate *priv = wb_image_button_get_instance_private (self);

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
    priv->pixbuf = gdk_pixbuf_new_from_stream (stream, NULL, &error);
    if (error != NULL)
    {
        g_warning ("Unable to create pixbuf: %s",
                   error->message);
        g_clear_error (&error);
    }

    /* Scale the image into thumbnail (150*150) */
    if (priv->type == WB_MEDIA_TYPE_IMAGE)
    {
        gdouble scale;
        gint width, height;
        gint offset;
        GdkPixbuf *scaled_pixbuf;

        width = gdk_pixbuf_get_width (priv->pixbuf);
        height = gdk_pixbuf_get_height (priv->pixbuf);

        /* If the image is large enough, cropped the central square part
         * of the image and scale it down to 150px by 150px */
        if (width > priv->width && height > priv->height)
        {
            scaled_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
                                            gdk_pixbuf_get_has_alpha (priv->pixbuf),
                                            8, priv->width, priv->height);

            if (width <= height)
            {
                scale = (gdouble) priv->width / width;
                offset = (height - width) * scale / -2;

                gdk_pixbuf_scale (priv->pixbuf, scaled_pixbuf,
                                  0, 0, priv->width, priv->height,
                                  0, offset, scale, scale,
                                  GDK_INTERP_BILINEAR);
            }
            else
            {
                scale = (gdouble) priv->height / height;
                offset = (width - height) * scale / -2;

                gdk_pixbuf_scale (priv->pixbuf, scaled_pixbuf,
                                  0, 0, priv->width, priv->height,
                                  offset, 0, scale, scale,
                                  GDK_INTERP_BILINEAR);
            }
        }
        else
        {
            scaled_pixbuf = gdk_pixbuf_scale_simple (priv->pixbuf,
                                                     priv->width,
                                                     priv->height,
                                                     GDK_INTERP_BILINEAR);
        }

        gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image), scaled_pixbuf);

        g_object_unref (scaled_pixbuf);
    }
    else
    {
        gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image), priv->pixbuf);
    }
}

static void
wb_image_button_constructed (GObject *object)
{
    SoupMessage *msg;
    WbImageButton *self = WB_IMAGE_BUTTON (object);

    WbImageButtonPrivate *priv = wb_image_button_get_instance_private (self);

    msg = soup_message_new (SOUP_METHOD_GET, priv->uri);
    soup_session_queue_message (priv->session, msg,
                                on_message_complete, self);

    G_OBJECT_CLASS (wb_image_button_parent_class)->constructed (object);
}

static void
wb_image_button_finalize (GObject *object)
{
    WbImageButton *self = WB_IMAGE_BUTTON (object);
    WbImageButtonPrivate *priv = wb_image_button_get_instance_private (self);

    g_clear_object (&priv->session);
    g_free (priv->uri);
    if (priv->pixbuf != NULL)
    {
        g_object_unref (priv->pixbuf);
    }

    G_OBJECT_CLASS (wb_image_button_parent_class)->finalize (object);
}

static void
wb_image_button_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    WbImageButton *self = WB_IMAGE_BUTTON (object);
    WbImageButtonPrivate *priv = wb_image_button_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_MEDIA_TYPE:
            g_value_set_enum (value, priv->type);
            break;
        case PROP_URI:
            g_value_set_string (value, priv->uri);
            break;
        case PROP_NTH_MEDIA:
            g_value_set_int (value, priv->nth_media);
            break;
        case PROP_WIDTH:
            g_value_set_int (value, priv->width);
            break;
        case PROP_HEIGHT:
            g_value_set_int (value, priv->height);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
wb_image_button_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    WbImageButton *self = WB_IMAGE_BUTTON (object);
    WbImageButtonPrivate *priv = wb_image_button_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_MEDIA_TYPE:
            priv->type = g_value_get_enum (value);
            break;
        case PROP_URI:
            g_free (priv->uri);
            priv->uri = g_value_dup_string (value);
            break;
        case PROP_NTH_MEDIA:
            priv->nth_media = g_value_get_int (value);
            break;
        case PROP_WIDTH:
            priv->width = g_value_get_int (value);
            break;
        case PROP_HEIGHT:
            priv->height = g_value_get_int (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
wb_image_button_class_init (WbImageButtonClass *klass)
{
		GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->constructed = wb_image_button_constructed;
		object_class->finalize = wb_image_button_finalize;
		object_class->get_property = wb_image_button_get_property;
		object_class->set_property = wb_image_button_set_property;

    obj_properties[PROP_URI] = g_param_spec_string ("uri", "URI",
                                                    "URI for the image or video",
                                                    NULL,
                                                    G_PARAM_READWRITE |
                                                    G_PARAM_CONSTRUCT_ONLY |
                                                    G_PARAM_STATIC_STRINGS);
    obj_properties[PROP_MEDIA_TYPE] = g_param_spec_enum ("media-type",
                                                         "Media type",
                                                         "Media type",
                                                         WB_TYPE_MEDIA_TYPE,
                                                         WB_MEDIA_TYPE_IMAGE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY |
                                                         G_PARAM_STATIC_STRINGS);
    obj_properties[PROP_NTH_MEDIA] = g_param_spec_int ("nth-media",
                                                       "Nth Media",
                                                       "Nth Media of the multimedia grid",
                                                       1, 9, 1,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY |
                                                       G_PARAM_STATIC_STRINGS);
    obj_properties[PROP_WIDTH] = g_param_spec_int ("width",
                                                   "Width",
                                                   "Width of the thumbnail image",
                                                   50, 300, 150,
                                                   G_PARAM_READWRITE |
                                                   G_PARAM_CONSTRUCT_ONLY |
                                                   G_PARAM_STATIC_STRINGS);
    obj_properties[PROP_HEIGHT] = g_param_spec_int ("height",
                                                   "Height",
                                                   "Height of the thumbnail image",
                                                   50, 300, 150,
                                                   G_PARAM_READWRITE |
                                                   G_PARAM_CONSTRUCT_ONLY |
                                                   G_PARAM_STATIC_STRINGS);
    g_object_class_install_properties (object_class, N_PROPS, obj_properties);
}

static void
wb_image_button_init (WbImageButton *self)
{
    GtkStyleContext *context;
    WbImageButtonPrivate *priv;

    priv = wb_image_button_get_instance_private (self);

    priv->uri = NULL;
    priv->pixbuf = NULL;
    priv->session = soup_session_new ();

    priv->image = gtk_image_new_from_pixbuf (NULL);
    gtk_button_set_image (GTK_BUTTON (self), priv->image);

    context = gtk_widget_get_style_context (GTK_WIDGET (self));
    gtk_style_context_add_class (context, "flat");
    gtk_style_context_add_class (context, "ImageButton");
}

/**
 * wb_image_button_new:
 *
 * Create a new #WbImageButton.
 *
 * Returns: (transfer full): a newly created #WbImageButton
 */
WbImageButton*
wb_image_button_new (WbMediaType type,
                     const gchar *uri,
                     gint nth_media,
                     gint width,
                     gint height)
{
    return g_object_new (WB_TYPE_IMAGE_BUTTON,
                         "media-type", type,
                         "uri", uri,
                         "nth-media", nth_media,
                         "width", width,
                         "height", height,
                         NULL);
}
