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

#include "gw-timeline-row.h"

struct _GwTimelineRow
{
    /*< private >*/
    GtkListBoxRow parent_instance;
};

typedef struct
{
} GwTimelineRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GwTimelineRow, gw_timeline_row, GTK_TYPE_LIST_BOX_ROW)

static void
gw_timeline_row_class_init (GwTimelineRowClass *klass)
{
}

static void
gw_timeline_row_init (GwTimelineRow *self)
{
}

/**
 * gw_timeline_row_new:
 *
 * Create a new #GwTimelineRow.
 *
 * Returns: (transfer full): a newly created #GwTimelineRow
 */
GwTimelineRow *
gw_timeline_row_new (void)
{
    return g_object_new (GW_TYPE_TIMELINE_ROW, NULL);
}
