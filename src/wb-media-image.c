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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <libsoup/soup.h>

#include "wb-enums.h"
#include "wb-media-image.h"

enum
{
    PROP_0,
    PROP_URI,
    PROP_MEDIA_TYPE,
    PROP_WIDTH,
    PROP_HEIGHT,
    N_PROPS
};

struct _WbMediaImage
{
		GtkImage parent_instance;
};

typedef struct
{
    gchar *uri;
    gint width;
    gint height;
    GdkPixbuf *pixbuf;
    SoupSession *session;
    WbMediaType type;
} WbMediaImagePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbMediaImage, wb_media_image, GTK_TYPE_IMAGE)

WbMediaType
wb_media_image_get_media_type (WbMediaImage *self)
{
    WbMediaImagePrivate *priv = wb_media_image_get_instance_private (self);

    return priv->type;
}

GdkPixbuf *
wb_media_image_get_pixbuf (WbMediaImage *self)
{
    WbMediaImagePrivate *priv = wb_media_image_get_instance_private (self);

    return priv->pixbuf;
}

static GParamSpec *obj_properties [N_PROPS] = { NULL, };

static void
on_message_complete (SoupSession *session,
                     SoupMessage *msg,
                     gpointer user_data)
{
    GError *error = NULL;
    GInputStream *stream;
    WbMediaImage *self = WB_MEDIA_IMAGE (user_data);
    WbMediaImagePrivate *priv = wb_media_image_get_instance_private (self);

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
        scaled_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
                                        gdk_pixbuf_get_has_alpha (priv->pixbuf),
                                        8, priv->width, priv->height);

        /* If the image is large enough, cropped the central square part
         * of the image and scale it down to 150px by 150px */
        if (width > priv->width && height > priv->height)
        {
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

        gtk_image_set_from_pixbuf (GTK_IMAGE (self), scaled_pixbuf);

        g_object_unref (scaled_pixbuf);
    }
    else
    {
        gtk_image_set_from_pixbuf (GTK_IMAGE (self), priv->pixbuf);
    }

    g_input_stream_close (stream, NULL, &error);
    if (error != NULL)
    {
        g_warning ("Unable to close the input stream: %s",
                   error->message);
        g_clear_error (&error);
    }
}

static void
wb_media_image_constructed (GObject *object)
{
    SoupMessage *msg;
    WbMediaImage *self = WB_MEDIA_IMAGE (object);
    WbMediaImagePrivate *priv = wb_media_image_get_instance_private (self);

    msg = soup_message_new (SOUP_METHOD_GET, priv->uri);
    soup_session_queue_message (priv->session, msg,
                                on_message_complete, GTK_WIDGET (self));

    G_OBJECT_CLASS (wb_media_image_parent_class)->constructed (object);
}

static void
wb_media_image_finalize (GObject *object)
{
    WbMediaImage *self = WB_MEDIA_IMAGE (object);
    WbMediaImagePrivate *priv = wb_media_image_get_instance_private (self);

    g_clear_object (&priv->session);
    g_free (priv->uri);
    if (priv->pixbuf != NULL)
    {
        g_object_unref (priv->pixbuf);
    }

    G_OBJECT_CLASS (wb_media_image_parent_class)->finalize (object);
}

static void
wb_media_image_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    WbMediaImage *self = WB_MEDIA_IMAGE (object);
    WbMediaImagePrivate *priv = wb_media_image_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_MEDIA_TYPE:
            g_value_set_enum (value, priv->type);
            break;
        case PROP_URI:
            g_value_set_string (value, priv->uri);
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
wb_media_image_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    WbMediaImage *self = WB_MEDIA_IMAGE (object);
    WbMediaImagePrivate *priv = wb_media_image_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_MEDIA_TYPE:
            priv->type = g_value_get_enum (value);
            break;
        case PROP_URI:
            g_free (priv->uri);
            priv->uri = g_value_dup_string (value);
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
wb_media_image_class_init (WbMediaImageClass *klass)
{
		GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->constructed = wb_media_image_constructed;
		object_class->finalize = wb_media_image_finalize;
		object_class->get_property = wb_media_image_get_property;
		object_class->set_property = wb_media_image_set_property;

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
wb_media_image_init (WbMediaImage *self)
{
    WbMediaImagePrivate *priv;

    priv = wb_media_image_get_instance_private (self);

    priv->uri = NULL;
    priv->pixbuf = NULL;
    priv->session = soup_session_new ();
}

/**
 * wb_media_image_new:
 *
 * Create a new #WbMediaImage.
 *
 * Returns: (transfer full): a newly created #WbMediaImage
 */
WbMediaImage *
wb_media_image_new (WbMediaType type,
                    const gchar *uri,
                    gint width,
                    gint height)
{
    return g_object_new (WB_TYPE_MEDIA_IMAGE,
                         "media-type", type,
                         "uri", uri,
                         "width", width,
                         "height", height,
                         NULL);
}
