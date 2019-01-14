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
} GwTimelineListPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GwTimelineList, gw_timeline_list, GTK_TYPE_BOX)

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
    gtk_widget_init_template (GTK_WIDGET (self));
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
