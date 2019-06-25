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

#include <gtk/gtk.h>

#include "wb-name-button.h"

struct _WbNameButton
{
		GtkButton parent_instance;
};

G_DEFINE_TYPE (WbNameButton, wb_name_button, GTK_TYPE_BUTTON)

void
wb_name_button_set_text (WbNameButton *self,
                         const gchar *text)
{
    GtkWidget *label;

    g_return_if_fail (WB_IS_NAME_BUTTON (self));

    label = gtk_label_new (text);
    gtk_widget_show (label);
    gtk_container_add (GTK_CONTAINER (self), label);
}

static void
wb_name_button_class_init (WbNameButtonClass *klass)
{
}

static void
wb_name_button_init (WbNameButton *self)
{
    GtkStyleContext *context;

    context = gtk_widget_get_style_context (GTK_WIDGET (self));
    gtk_style_context_add_class (context, "name-button");
}

/**
 * wb_name_button_new:
 *
 * Create a new #WbNameButton.
 *
 * Returns: (transfer full): a newly created #WbNameButton
 */
WbNameButton *
wb_name_button_new (void)
{
    return g_object_new (WB_TYPE_NAME_BUTTON, NULL);
}
