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
#include "gw-timeline-row.h"

enum
{
    PROP_0,
    PROP_POST_ITEM,
    N_PROPERTIES
};

struct _GwTimelineRow
{
    /*< private >*/
    GtkListBoxRow parent_instance;
};

typedef struct
{
    GwPostItem *post_item;
} GwTimelineRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GwTimelineRow, gw_timeline_row, GTK_TYPE_LIST_BOX_ROW)

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
gw_timeline_row_constructed (GObject *object)
{
}

static void
gw_timeline_row_finalize (GObject *object)
{
}

static void
gw_timeline_row_get_property (GObject *object,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
    GwTimelineRow *self = GW_TIMELINE_ROW (object);
    GwTimelineRowPrivate *priv = gw_timeline_row_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_POST_ITEM:
            g_value_set_pointer (value, priv->post_item);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gw_timeline_row_set_property (GObject *object,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
    GwTimelineRow *self = GW_TIMELINE_ROW (object);
    GwTimelineRowPrivate *priv = gw_timeline_row_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_POST_ITEM:
            priv->post_item = g_value_get_pointer (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gw_timeline_row_class_init (GwTimelineRowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructed = gw_timeline_row_constructed;
    gobject_class->finalize = gw_timeline_row_finalize;
    gobject_class->get_property = gw_timeline_row_get_property;
    gobject_class->set_property = gw_timeline_row_set_property;

    obj_properties[PROP_POST_ITEM] = g_param_spec_pointer ("post-item", "item",
                                                           "Post item for each row",
                                                           G_PARAM_READWRITE |
                                                           G_PARAM_CONSTRUCT_ONLY |
                                                           G_PARAM_STATIC_STRINGS);
    g_object_class_install_properties (gobject_class, N_PROPERTIES,
                                       obj_properties);
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
gw_timeline_row_new (GwPostItem *post_item)
{
    return g_object_new (GW_TYPE_TIMELINE_ROW,
                         "post-item", post_item,
                         NULL);
}
