/*
 *  GNOME Weibo - view and compose weibo
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

#include "gw-enums.h"
#include "gw-image-button.h"

enum
{
    PROP_0,
    PROP_URI,
    PROP_MEDIA_TYPE,
    N_PROPS
};

struct _GwImageButton
{
    /*< private >*/
    GtkButton parent_instance;
};

typedef struct
{
    gchar *uri;
    GtkWidget *image;
    GwMediaType type;
} GwImageButtonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GwImageButton, gw_image_button, GTK_TYPE_BUTTON)

static GParamSpec *obj_properties [N_PROPS] = { NULL, };

static void
on_message_complete (SoupSession *session,
                     SoupMessage *msg,
                     gpointer user_data)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    GInputStream *stream;
    GwImageButton *self = GW_IMAGE_BUTTON (user_data);
    GwImageButtonPrivate *priv = gw_image_button_get_instance_private (self);

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

    gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image), pixbuf);

    g_input_stream_close (stream, NULL, &error);
    if (error != NULL)
    {
        g_warning ("Unable to close the input stream: %s",
                   error->message);
        g_clear_error (&error);
    }
}

static void
gw_image_button_constructed (GObject *object)
{
    SoupMessage *msg;
    SoupSession *session;
    GwImageButton *self = GW_IMAGE_BUTTON (object);
    GwImageButtonPrivate *priv = gw_image_button_get_instance_private (self);

    msg = soup_message_new (SOUP_METHOD_GET, priv->uri);
    session = soup_session_new ();
    soup_session_queue_message (session, msg,
                                on_message_complete, GTK_WIDGET (self));

    G_OBJECT_CLASS (gw_image_button_parent_class)->constructed (object);
}

static void
gw_image_button_finalize (GObject *object)
{
    GwImageButton *self = GW_IMAGE_BUTTON (object);
    GwImageButtonPrivate *priv = gw_image_button_get_instance_private (self);

    g_free (priv->uri);

    G_OBJECT_CLASS (gw_image_button_parent_class)->finalize (object);
}

static void
gw_image_button_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    GwImageButton *self = GW_IMAGE_BUTTON (object);
    GwImageButtonPrivate *priv = gw_image_button_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_MEDIA_TYPE:
            g_value_set_enum (value, priv->type);
            break;
        case PROP_URI:
            g_value_set_string (value, priv->uri);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gw_image_button_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    GwImageButton *self = GW_IMAGE_BUTTON (object);
    GwImageButtonPrivate *priv = gw_image_button_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_MEDIA_TYPE:
            priv->type = g_value_get_enum (value);
            break;
        case PROP_URI:
            g_free (priv->uri);
            priv->uri = g_value_dup_string (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gw_image_button_class_init (GwImageButtonClass *klass)
{
		GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->constructed = gw_image_button_constructed;
		object_class->finalize = gw_image_button_finalize;
		object_class->get_property = gw_image_button_get_property;
		object_class->set_property = gw_image_button_set_property;

    obj_properties[PROP_URI] = g_param_spec_string ("uri", "URI",
                                                    "URI for the image or video",
                                                    NULL,
                                                    G_PARAM_READWRITE |
                                                    G_PARAM_CONSTRUCT_ONLY |
                                                    G_PARAM_STATIC_STRINGS);
    obj_properties[PROP_MEDIA_TYPE] = g_param_spec_enum ("media-type",
                                                         "Media type",
                                                         "Media type",
                                                         GW_TYPE_MEDIA_TYPE,
                                                         GW_MEDIA_TYPE_IMAGE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY |
                                                         G_PARAM_STATIC_STRINGS);
    g_object_class_install_properties (object_class, N_PROPS, obj_properties);
}

static void
gw_image_button_init (GwImageButton *self)
{
    GtkStyleContext *context;
    GwImageButtonPrivate *priv;

    priv = gw_image_button_get_instance_private (self);

    priv->uri = NULL;

    priv->image = gtk_image_new_from_pixbuf (NULL);
    gtk_button_set_image (GTK_BUTTON (self), priv->image);

    context = gtk_widget_get_style_context (GTK_WIDGET (self));
    gtk_style_context_add_class (context, "flat");
}

/**
 * gw_image_button_new:
 *
 * Create a new #GwImageButton.
 *
 * Returns: (transfer full): a newly created #GwImageButton
 */
GtkWidget *
gw_image_button_new (GwMediaType type,
                     const gchar *uri)
{
    return g_object_new (GW_TYPE_IMAGE_BUTTON,
                         "media-type", type,
                         "uri", uri,
                         NULL);
}
