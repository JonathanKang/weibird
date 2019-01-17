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
    GtkWidget *hbox;
    GtkWidget *main_box;
    GtkWidget *name_label;
    GtkWidget *text_label;
    GtkWidget *time_label;
    GwTimelineRow *row = GW_TIMELINE_ROW (object);
    GwTimelineRowPrivate *priv = gw_timeline_row_get_instance_private (row);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_start (main_box, 6);
    gtk_widget_set_margin_end (main_box, 6);
    gtk_widget_set_margin_top (main_box, 6);
    gtk_widget_set_margin_bottom (main_box, 6);

    name_label = gtk_label_new (priv->post_item->user->name);
    gtk_widget_set_halign (name_label, GTK_ALIGN_START);
    gtk_box_pack_start (GTK_BOX (hbox), name_label, TRUE, TRUE, 0);

    time_label = gtk_label_new (priv->post_item->created_at);
    gtk_widget_set_halign (time_label, GTK_ALIGN_END);
    gtk_box_pack_end (GTK_BOX (hbox), time_label, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (main_box), hbox, TRUE, TRUE, 0);

    text_label = gtk_label_new (priv->post_item->text);
    gtk_widget_set_halign (text_label, GTK_ALIGN_START);
    gtk_label_set_line_wrap (GTK_LABEL (text_label), TRUE);
    gtk_box_pack_end (GTK_BOX (main_box), text_label, TRUE, TRUE, 0);

    gtk_container_add (GTK_CONTAINER (row), main_box);
    gtk_widget_show_all (GTK_WIDGET (row));

    G_OBJECT_CLASS (gw_timeline_row_parent_class)->constructed (object);
}

static void
gw_timeline_row_finalize (GObject *object)
{
    GwTimelineRow *self = GW_TIMELINE_ROW (object);
    GwTimelineRowPrivate *priv = gw_timeline_row_get_instance_private (self);

    g_free (priv->post_item->created_at);
    g_free (priv->post_item->idstr);
    g_free (priv->post_item->text);
    g_free (priv->post_item->source);
    g_free (priv->post_item->thumbnail_pic);
    g_free (priv->post_item->user->idstr);
    g_free (priv->post_item->user->name);
    g_free (priv->post_item->user->location);
    g_free (priv->post_item->user->description);
    g_free (priv->post_item->user->url);
    g_free (priv->post_item->user->profile_image_url);
    g_free (priv->post_item->user->gender);
    g_free (priv->post_item->user->created_at);
    g_free (priv->post_item->user);
    g_free (priv->post_item);
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
