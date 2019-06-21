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

#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <libsoup/soup.h>

#include "wb-avatar-widget.h"
#include "wb-util.h"

struct _WbAvatarWidget
{
		GtkWidget parent_instance;
};

typedef struct
{
    cairo_surface_t *surface;
    gint width;
    gint height;
    GdkPixbuf *pixbuf;
    GdkPixbuf *scaled_pixbuf;
    GdkWindow *event_window;
} WbAvatarWidgetPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbAvatarWidget, wb_avatar_widget, GTK_TYPE_WIDGET)

static void on_message_complete (SoupSession *session,
                                 SoupMessage *msg,
                                 gpointer user_data);

void
wb_avatar_widget_setup (WbAvatarWidget *self,
                        const gchar *uri)
{
    SoupMessage *msg;
    WbAvatarWidgetPrivate *priv = wb_avatar_widget_get_instance_private (self);

    priv->width = 50;
    priv->height = 50;

    msg = soup_message_new (SOUP_METHOD_GET, uri);
    soup_session_queue_message (SOUPSESSION, msg,
                                on_message_complete, self);
}

static void
on_message_complete (SoupSession *session,
                     SoupMessage *msg,
                     gpointer user_data)
{
    g_autoptr(GInputStream) stream = NULL;
    GError *error = NULL;
    WbAvatarWidget *self = WB_AVATAR_WIDGET (user_data);
    WbAvatarWidgetPrivate *priv = wb_avatar_widget_get_instance_private (self);

    if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code) &&
        msg->status_code != SOUP_STATUS_CANCELLED)
    {
        g_warning ("Failed to fetch avatar: %d %s.\n",
                   msg->status_code, msg->reason_phrase);

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

    priv->surface = gdk_cairo_surface_create_from_pixbuf (priv->pixbuf, 0, NULL);
}

static gboolean
wb_avatar_widget_draw (GtkWidget *widget,
                       cairo_t *cr)
{
    WbAvatarWidget *self;
    WbAvatarWidgetPrivate *priv;

    self = WB_AVATAR_WIDGET (widget);
    priv = wb_avatar_widget_get_instance_private (self);

    if (priv->surface == NULL)
    {
        return GDK_EVENT_PROPAGATE;
    }

    cairo_set_source_surface (cr, priv->surface, 0, 0);
    cairo_paint (cr);

    return GDK_EVENT_PROPAGATE;
}

static GtkSizeRequestMode
wb_avatar_widget_get_request_mode (GtkWidget *widget)
{
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
wb_avatar_widget_get_preferred_height (GtkWidget *widget,
                                       gint *minimum_height,
                                       gint *natural_height)
{
    WbAvatarWidget *self;
    WbAvatarWidgetPrivate *priv;

    self = WB_AVATAR_WIDGET (widget);
    priv = wb_avatar_widget_get_instance_private (self);

    *minimum_height = priv->height;
    *natural_height = priv->height;
}

static void
wb_avatar_widget_get_preferred_width (GtkWidget *widget,
                                      gint *minimum_width,
                                      gint *natural_width)
{
    WbAvatarWidget *self;
    WbAvatarWidgetPrivate *priv;

    self = WB_AVATAR_WIDGET (widget);
    priv = wb_avatar_widget_get_instance_private (self);

    *minimum_width = priv->width;
    *natural_width = priv->width;
}

static void
wb_avatar_widget_realize (GtkWidget *widget)
{
    GdkWindow *window;
    GdkWindowAttr attr = {};
    GdkWindowAttributesType attr_mask;
    GtkAllocation allocation;
    WbAvatarWidget *self;
    WbAvatarWidgetPrivate *priv;

    self = WB_AVATAR_WIDGET (widget);
    priv = wb_avatar_widget_get_instance_private (self);

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
wb_avatar_widget_unrealize (GtkWidget *widget)
{
    WbAvatarWidget *self;
    WbAvatarWidgetPrivate *priv;

    self = WB_AVATAR_WIDGET (widget);
    priv = wb_avatar_widget_get_instance_private (self);

    if (priv->event_window != NULL)
    {
        gtk_widget_unregister_window (widget, priv->event_window);
        gdk_window_destroy (priv->event_window);
        priv->event_window = NULL;
    }

    GTK_WIDGET_CLASS (wb_avatar_widget_parent_class)->unrealize (widget);
}

static void
wb_avatar_widget_map (GtkWidget *widget)
{
    WbAvatarWidget *self;
    WbAvatarWidgetPrivate *priv;

    self = WB_AVATAR_WIDGET (widget);
    priv = wb_avatar_widget_get_instance_private (self);

    GTK_WIDGET_CLASS (wb_avatar_widget_parent_class)->map (widget);

    if (priv->event_window != NULL)
    {
        gdk_window_show (priv->event_window);
    }
}

static void
wb_avatar_widget_unmap (GtkWidget *widget)
{
    WbAvatarWidget *self;
    WbAvatarWidgetPrivate *priv;

    self = WB_AVATAR_WIDGET (widget);
    priv = wb_avatar_widget_get_instance_private (self);

    if (priv->event_window != NULL)
    {
        gdk_window_hide (priv->event_window);
    }

    GTK_WIDGET_CLASS (wb_avatar_widget_parent_class)->unmap (widget);
}

static void
wb_avatar_widget_finalize (GObject *object)
{
    WbAvatarWidget *self = (WbAvatarWidget *)object;
    WbAvatarWidgetPrivate *priv = wb_avatar_widget_get_instance_private (self);

    if (priv->pixbuf != NULL)
    {
        g_object_unref (priv->pixbuf);
    }
    if (priv->scaled_pixbuf != NULL)
    {
        g_object_unref (priv->scaled_pixbuf);
    }

    G_OBJECT_CLASS (wb_avatar_widget_parent_class)->finalize (object);
}

static void
wb_avatar_widget_class_init (WbAvatarWidgetClass *klass)
{
		GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

		object_class->finalize = wb_avatar_widget_finalize;

    widget_class->draw = wb_avatar_widget_draw;
    widget_class->get_request_mode = wb_avatar_widget_get_request_mode;
    widget_class->get_preferred_height = wb_avatar_widget_get_preferred_height;
    widget_class->get_preferred_width = wb_avatar_widget_get_preferred_width;
    widget_class->realize = wb_avatar_widget_realize;
    widget_class->unrealize = wb_avatar_widget_unrealize;
    widget_class->map = wb_avatar_widget_map;
    widget_class->unmap = wb_avatar_widget_unmap;
}

static void
wb_avatar_widget_init (WbAvatarWidget *self)
{
    WbAvatarWidgetPrivate *priv;

    gtk_widget_set_can_focus (GTK_WIDGET (self), TRUE);
    gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);
    gtk_widget_add_events (GTK_WIDGET (self),
                           GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);

    priv = wb_avatar_widget_get_instance_private (self);

    priv->event_window = NULL;
    priv->pixbuf = NULL;
    priv->scaled_pixbuf = NULL;
    priv->surface = NULL;
}

/**
 * wb_avatar_widget_new:
 *
 * Create a new #WbAvatarWidget.
 *
 * Returns: (transfer full): a newly created #WbAvatarWidget
 */
WbAvatarWidget *
wb_avatar_widget_new (void)
{
    return g_object_new (WB_TYPE_AVATAR_WIDGET, NULL);
}
