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

#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <libsoup/soup.h>

#include "wb-enums.h"
#include "wb-image-button.h"
#include "wb-util.h"

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

enum
{
    CLICKED,
    LAST_SIGNAL
};

struct _WbImageButton
{
    /*< private >*/
    GtkWidget parent_instance;
};

typedef struct
{
    cairo_surface_t *surface;
    PangoLayout *layout;
    gboolean media_loaded;
    gchar *uri;
    gint nth_media;
    gint width;
    gint height;
    GdkPixbuf *pixbuf;
    GdkPixbuf *scaled_pixbuf;
    GdkWindow *event_window;
    GtkWidget *image;
    WbMediaType type;
} WbImageButtonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbImageButton, wb_image_button, GTK_TYPE_WIDGET)

static GParamSpec *obj_properties [N_PROPS] = { NULL, };
static guint signals[LAST_SIGNAL] = { 0 };

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

static gboolean
wb_image_button_draw (GtkWidget *widget,
                      cairo_t *cr)
{
    gint widget_width;
    gint widget_height;
    WbImageButton *self;
    WbImageButtonPrivate *priv;

    self = WB_IMAGE_BUTTON (widget);
    priv = wb_image_button_get_instance_private (self);

    widget_width = gtk_widget_get_allocated_width (widget);
    widget_height = gtk_widget_get_allocated_height (widget);

    if (priv->surface != NULL && priv->media_loaded)
    {
        cairo_set_source_surface (cr, priv->surface, 0, 0);
        cairo_paint (cr);
    }
    else
    {
        gdouble layout_x;
        gdouble layout_y;
        gint layout_width;
        gint layout_height;
        GtkStyleContext *context;

        context = gtk_widget_get_style_context (widget);

        pango_layout_get_size (priv->layout, &layout_width, &layout_height);
        layout_x = widget_width / 2.0 - layout_width / PANGO_SCALE / 2.0;
        layout_y = widget_height / 2.0 - layout_height / PANGO_SCALE / 2.0;

        gtk_render_layout (context, cr, layout_x, layout_y, priv->layout);
    }

    return GDK_EVENT_PROPAGATE;
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

    priv->media_loaded = TRUE;

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

        width = gdk_pixbuf_get_width (priv->pixbuf);
        height = gdk_pixbuf_get_height (priv->pixbuf);

        /* If the image is large enough, cropped the central square part
         * of the image and scale it down to 150px by 150px */
        if (width > priv->width && height > priv->height)
        {
            priv->scaled_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
                                                  gdk_pixbuf_get_has_alpha (priv->pixbuf),
                                                  8, priv->width, priv->height);

            if (width <= height)
            {
                scale = (gdouble) priv->width / width;
                offset = (height - width) * scale / -2;

                gdk_pixbuf_scale (priv->pixbuf, priv->scaled_pixbuf,
                                  0, 0, priv->width, priv->height,
                                  0, offset, scale, scale,
                                  GDK_INTERP_BILINEAR);
            }
            else
            {
                scale = (gdouble) priv->height / height;
                offset = (width - height) * scale / -2;

                gdk_pixbuf_scale (priv->pixbuf, priv->scaled_pixbuf,
                                  0, 0, priv->width, priv->height,
                                  offset, 0, scale, scale,
                                  GDK_INTERP_BILINEAR);
            }
        }
        else
        {
            priv->scaled_pixbuf = gdk_pixbuf_scale_simple (priv->pixbuf,
                                                           priv->width,
                                                           priv->height,
                                                           GDK_INTERP_BILINEAR);
        }

        priv->surface = gdk_cairo_surface_create_from_pixbuf (priv->scaled_pixbuf,
                                                              0, NULL);
    }
    else
    {
        priv->surface = gdk_cairo_surface_create_from_pixbuf (priv->pixbuf,
                                                              0, NULL);
    }

    gtk_widget_queue_draw (GTK_WIDGET (self));
}

static gboolean
wb_image_button_button_press_event (GtkWidget *widget,
                                    GdkEventButton *event)
{
    WbImageButton *self;

    self = WB_IMAGE_BUTTON (widget);

    if (event->button == GDK_BUTTON_PRIMARY)
    {
        g_signal_emit (self, signals[CLICKED], 0);

        return GDK_EVENT_STOP;
    }

    return GDK_EVENT_PROPAGATE;
}

static gboolean
wb_image_button_key_press_event (GtkWidget *widget,
                                 GdkEventKey *event)
{
    WbImageButton *self;

    self = WB_IMAGE_BUTTON (widget);

    if (event->keyval == GDK_KEY_Return ||
        event->keyval == GDK_KEY_KP_Enter)
    {
        g_signal_emit (self, signals[CLICKED], 0);

        return GDK_EVENT_STOP;
    }

    return GDK_EVENT_PROPAGATE;
}

static GtkSizeRequestMode
wb_image_button_get_request_mode (GtkWidget *widget)
{
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
wb_image_button_get_preferred_height (GtkWidget *widget,
                                      gint *minimum_height,
                                      gint *natural_height)
{
    WbImageButton *self;
    WbImageButtonPrivate *priv;

    self = WB_IMAGE_BUTTON (widget);
    priv = wb_image_button_get_instance_private (self);

    *minimum_height = priv->height;
    *natural_height = priv->height;
}

static void
wb_image_button_get_preferred_width (GtkWidget *widget,
                                     gint *minimum_width,
                                     gint *natural_width)
{
    WbImageButton *self;
    WbImageButtonPrivate *priv;

    self = WB_IMAGE_BUTTON (widget);
    priv = wb_image_button_get_instance_private (self);

    *minimum_width = priv->width;
    *natural_width = priv->width;
}

static void
wb_image_button_realize (GtkWidget *widget)
{
    GdkWindow *window;
    GdkWindowAttr attr = {};
    GdkWindowAttributesType attr_mask;
    GtkAllocation allocation;
    WbImageButton *self;
    WbImageButtonPrivate *priv;

    self = WB_IMAGE_BUTTON (widget);
    priv = wb_image_button_get_instance_private (self);

    gtk_widget_set_realized (widget, TRUE);

    window = gtk_widget_get_parent_window (widget);
    gtk_widget_set_window (widget, window);
    g_object_ref (window);

    gtk_widget_get_allocation (widget, &allocation);
    attr.x = allocation.x;
    attr.y = allocation.y;
    attr.width = allocation.width;
    attr.height = allocation.height;
    attr.window_type = GDK_WINDOW_CHILD;
    attr.visual = gtk_widget_get_visual (widget);
    attr.wclass = GDK_INPUT_ONLY;
    attr.event_mask = gtk_widget_get_events (widget) |
                      GDK_BUTTON_PRESS_MASK |
                      GDK_BUTTON_PRESS_MASK;
    attr_mask = GDK_WA_X | GDK_WA_Y;

    priv->event_window = gdk_window_new (window, &attr, attr_mask);
    gtk_widget_register_window (widget, priv->event_window);
}

static void
wb_image_button_unrealize (GtkWidget *widget)
{
    WbImageButton *self;
    WbImageButtonPrivate *priv;

    self = WB_IMAGE_BUTTON (widget);
    priv = wb_image_button_get_instance_private (self);

    if (priv->event_window != NULL)
    {
        gtk_widget_unregister_window (widget, priv->event_window);
        gdk_window_destroy (priv->event_window);
        priv->event_window = NULL;
    }

    GTK_WIDGET_CLASS (wb_image_button_parent_class)->unrealize (widget);
}

static void
wb_image_button_map (GtkWidget *widget)
{
    WbImageButton *self;
    WbImageButtonPrivate *priv;

    self = WB_IMAGE_BUTTON (widget);
    priv = wb_image_button_get_instance_private (self);

    GTK_WIDGET_CLASS (wb_image_button_parent_class)->map (widget);

    if (priv->event_window != NULL)
    {
        gdk_window_show (priv->event_window);
    }
}

static void
wb_image_button_unmap (GtkWidget *widget)
{
    WbImageButton *self;
    WbImageButtonPrivate *priv;

    self = WB_IMAGE_BUTTON (widget);
    priv = wb_image_button_get_instance_private (self);

    if (priv->event_window != NULL)
    {
        gdk_window_hide (priv->event_window);
    }

    GTK_WIDGET_CLASS (wb_image_button_parent_class)->unmap (widget);
}

static void
wb_image_button_constructed (GObject *object)
{
    SoupMessage *msg;
    WbImageButton *self = WB_IMAGE_BUTTON (object);

    WbImageButtonPrivate *priv = wb_image_button_get_instance_private (self);

    msg = soup_message_new (SOUP_METHOD_GET, priv->uri);
    soup_session_queue_message (SOUPSESSION, msg,
                                on_message_complete, self);

    G_OBJECT_CLASS (wb_image_button_parent_class)->constructed (object);
}

static void
wb_image_button_finalize (GObject *object)
{
    WbImageButton *self = WB_IMAGE_BUTTON (object);
    WbImageButtonPrivate *priv = wb_image_button_get_instance_private (self);

    g_free (priv->uri);
    if (priv->pixbuf != NULL)
    {
        g_object_unref (priv->pixbuf);
    }
    if (priv->scaled_pixbuf != NULL)
    {
        g_object_unref (priv->scaled_pixbuf);
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
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->constructed = wb_image_button_constructed;
		object_class->finalize = wb_image_button_finalize;
		object_class->get_property = wb_image_button_get_property;
		object_class->set_property = wb_image_button_set_property;

    widget_class->button_press_event = wb_image_button_button_press_event;
    widget_class->draw = wb_image_button_draw;
    widget_class->get_request_mode = wb_image_button_get_request_mode;
    widget_class->get_preferred_height = wb_image_button_get_preferred_height;
    widget_class->get_preferred_width = wb_image_button_get_preferred_width;
    widget_class->key_press_event = wb_image_button_key_press_event;
    widget_class->realize = wb_image_button_realize;
    widget_class->unrealize = wb_image_button_unrealize;
    widget_class->map = wb_image_button_map;
    widget_class->unmap = wb_image_button_unmap;

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

    signals[CLICKED] = g_signal_new ("clicked",
                                     G_TYPE_FROM_CLASS (klass),
                                     G_SIGNAL_RUN_LAST,
                                     0,
                                     NULL,
                                     NULL,
                                     NULL,
                                     G_TYPE_NONE,
                                     0);
}

static void
wb_image_button_init (WbImageButton *self)
{
    WbImageButtonPrivate *priv;

    gtk_widget_set_can_focus (GTK_WIDGET (self), TRUE);
    gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);
    gtk_widget_add_events (GTK_WIDGET (self),
                           GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);

    priv = wb_image_button_get_instance_private (self);

    priv->event_window = NULL;
    priv->media_loaded = FALSE;
    priv->uri = NULL;
    priv->pixbuf = NULL;
    priv->scaled_pixbuf = NULL;
    priv->surface = NULL;
    priv->layout = gtk_widget_create_pango_layout (GTK_WIDGET (self), "...");
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
