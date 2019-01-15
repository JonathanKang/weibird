/*
 *  gnome weibo - view and compose weibo
 *  copyright (c) 2018 jonathan kang <jonathankang@gnome.org>.
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

#include "gw-timeline-list.h"

struct _GwTimelineList
{
    /*< private >*/
    GtkBox parent_instance;
};

typedef struct
{
    GtkListBox *timeline_list;
    GtkWidget *login_button;
} GwTimelineListPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GwTimelineList, gw_timeline_list, GTK_TYPE_BOX)

GtkWidget *
gw_timeline_list_get_login_button (GwTimelineList *self)
{
    GwTimelineListPrivate *priv;

    priv = gw_timeline_list_get_instance_private (self);

    return priv->login_button;
}

static GtkWidget *
gw_timeline_list_create_placeholder (GwTimelineList *self)
{
    GtkWidget *box;
    GtkWidget *label;
    gchar *markup;
    GwTimelineListPrivate *priv;

    priv = gw_timeline_list_get_instance_private (self);

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(box, TRUE);
    gtk_widget_set_vexpand(box, TRUE);

    priv->login_button = gtk_button_new_with_label ("Login");
    gtk_widget_set_halign (priv->login_button, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (box), priv->login_button, FALSE, TRUE, 0);

    label = gtk_label_new(NULL);
    markup = g_markup_printf_escaped("<big>%s</big>", "Click the button to log in.");
    gtk_label_set_markup (GTK_LABEL (label), markup);
    gtk_box_pack_end (GTK_BOX (box), label, TRUE, TRUE, 0);

    gtk_widget_show_all (box);

    g_free (markup);

    return box;
}

static void
gw_timeline_list_class_init (GwTimelineListClass *klass)
{
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/org/gnome/Weibo/gw-timeline-list.ui");
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  GwTimelineList, timeline_list);
}

static void
gw_timeline_list_init (GwTimelineList *self)
{
    GwTimelineListPrivate *priv;
    GtkWidget *placeholder;

    priv = gw_timeline_list_get_instance_private (self);
    placeholder = gw_timeline_list_create_placeholder (self);

    gtk_widget_init_template (GTK_WIDGET (self));

    gtk_list_box_set_placeholder (priv->timeline_list, placeholder);
}

/**
 * gw_timeline_list_new:
 *
 * Create a new #GwTimelineList.
 *
 * Returns: (transfer full): a newly created #GwTimelineList
 */
GwTimelineList *
gw_timeline_list_new (void)
{
    return g_object_new (GW_TYPE_TIMELINE_LIST, NULL);
}
