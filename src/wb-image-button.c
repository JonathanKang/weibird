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

#include "wb-image-button.h"

struct _WbImageButton
{
    /*< private >*/
    GtkButton parent_instance;
};

G_DEFINE_TYPE (WbImageButton, wb_image_button, GTK_TYPE_BUTTON)

static void
wb_image_button_class_init (WbImageButtonClass *klass)
{
}

static void
wb_image_button_init (WbImageButton *self)
{
    GtkStyleContext *context;

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
GtkWidget *
wb_image_button_new (void)
{
    return g_object_new (WB_TYPE_IMAGE_BUTTON, NULL);
}
