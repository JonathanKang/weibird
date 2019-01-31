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
#include "wb-multi-media-widget.h"
#include "wb-timeline-list.h"
#include "wb-timeline-row.h"
#include "wb-util.h"

enum
{
    PROP_0,
    PROP_POST_ITEM,
    PROP_RETWEET,
    N_PROPERTIES
};

struct _WbTimelineRow
{
    /*< private >*/
    GtkListBoxRow parent_instance;
};

typedef struct
{
    gboolean retweet;
    GtkWidget *main_box;
    GtkWidget *profile_image;
    GtkWidget *post_image;
    WbPostItem *post_item;
} WbTimelineRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbTimelineRow, wb_timeline_row, GTK_TYPE_LIST_BOX_ROW)

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

void
wb_timeline_row_insert_retweeted_item (WbTimelineRow *self,
                                       GtkWidget *retweeted_item)
{
    GtkStyleContext *context;
    WbTimelineRowPrivate *priv;

    priv = wb_timeline_row_get_instance_private (self);

    context = gtk_widget_get_style_context (retweeted_item);
    gtk_style_context_add_class (context, "retweet");

    gtk_box_pack_end (GTK_BOX (priv->main_box), retweeted_item, FALSE, FALSE, 0);
}

static void
wb_timeline_row_constructed (GObject *object)
{
    gchar *markup;
    GtkStyleContext *context;
    GtkWidget *hbox1;
    GtkWidget *hbox2;
    GtkWidget *vbox;
    GtkWidget *name_label;
    GtkWidget *pic_grid;
    GtkWidget *source_label;
    GtkWidget *text_label;
    GtkWidget *time_label;
    GtkWidget *likes_label;
    GtkWidget *comments_label;
    GtkWidget *reposts_label;
    WbTimelineRow *row = WB_TIMELINE_ROW (object);
    WbTimelineRowPrivate *priv = wb_timeline_row_get_instance_private (row);

    hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start (GTK_BOX (priv->main_box), hbox1, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (priv->main_box), hbox2, FALSE, FALSE, 0);

    if (!priv->retweet)
    {
        /* Profile image (50px by 50px), name, source and time */
        priv->profile_image = wb_image_button_new (WB_MEDIA_TYPE_AVATAR,
                                                   priv->post_item->user->profile_image_url,
                                                   50, 50);
        gtk_widget_set_halign (priv->profile_image, GTK_ALIGN_START);
        gtk_box_pack_start (GTK_BOX (hbox1), priv->profile_image,
                            FALSE, FALSE, 0);
    }

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (hbox1), vbox, FALSE, FALSE, 0);

    if (g_strcmp0 (priv->post_item->user->nickname, "") != 0)
    {
        name_label = gtk_label_new (priv->post_item->user->nickname);
    }
    else
    {
        name_label = gtk_label_new (priv->post_item->user->name);
    }
    gtk_widget_set_halign (name_label, GTK_ALIGN_START);
    gtk_box_pack_start (GTK_BOX (vbox), name_label, TRUE, TRUE, 0);

    if (!priv->retweet && g_strcmp0 (priv->post_item->source, "") != 0)
    {
        gtk_widget_set_valign (name_label, GTK_ALIGN_END);

        priv->post_item->source = wb_util_format_source_string (priv->post_item->source);
        source_label = gtk_label_new (NULL);
        context = gtk_widget_get_style_context (source_label);
        gtk_style_context_add_class (context, "dim-label");
        gtk_label_set_markup (GTK_LABEL (source_label), priv->post_item->source);
        gtk_widget_set_halign (source_label, GTK_ALIGN_START);
        gtk_widget_set_valign (source_label, GTK_ALIGN_START);
        gtk_box_pack_end (GTK_BOX (vbox), source_label, TRUE, TRUE, 0);
    }

    priv->post_item->created_at = wb_util_format_time_string (priv->post_item->created_at);
    time_label = gtk_label_new (priv->post_item->created_at);
    context = gtk_widget_get_style_context (time_label);
    gtk_style_context_add_class (context, "dim-label");
    gtk_widget_set_halign (time_label, GTK_ALIGN_END);
    gtk_box_pack_end (GTK_BOX (hbox1), time_label, FALSE, FALSE, 0);

    /* Post content */
    text_label = gtk_label_new (priv->post_item->text);
    gtk_widget_set_halign (text_label, GTK_ALIGN_START);
    gtk_label_set_line_wrap (GTK_LABEL (text_label), TRUE);
    gtk_box_pack_start (GTK_BOX (priv->main_box), text_label, FALSE, FALSE, 0);

    /* Post image */
    /* TODO: Add support for display the original picture individually */
    if (priv->post_item->picuri_array->len != 0)
    {
        pic_grid = wb_multi_media_widget_new (priv->post_item->picuri_array->len,
                                              priv->post_item->picuri_array);
        gtk_widget_set_halign (pic_grid, GTK_ALIGN_CENTER);
        gtk_grid_set_column_spacing (GTK_GRID (pic_grid), 0);
        gtk_grid_set_row_spacing (GTK_GRID (pic_grid), 0);
        gtk_box_pack_start (GTK_BOX (priv->main_box), pic_grid, FALSE, FALSE, 0);
    }

    /* Likes, comments and reposts count */
    likes_label = gtk_label_new (NULL);
    markup = g_markup_printf_escaped ("<b>%d</b> Likes",
                                      priv->post_item->attitudes_count);
    gtk_label_set_markup (GTK_LABEL (likes_label), markup);
    gtk_box_pack_start (GTK_BOX (hbox2), likes_label, FALSE, FALSE, 0);
    g_free (markup);

    comments_label = gtk_label_new (NULL);
    markup = g_markup_printf_escaped ("<b>%d</b> Comments",
                                      priv->post_item->comments_count);
    gtk_label_set_markup (GTK_LABEL (comments_label), markup);
    gtk_box_pack_start (GTK_BOX (hbox2), comments_label, FALSE, FALSE, 0);
    g_free (markup);

    reposts_label = gtk_label_new (NULL);
    markup = g_markup_printf_escaped ("<b>%d</b> Reposts",
                                      priv->post_item->reposts_count);
    gtk_label_set_markup (GTK_LABEL (reposts_label), markup);
    gtk_box_pack_start (GTK_BOX (hbox2), reposts_label, FALSE, FALSE, 0);
    g_free (markup);

    gtk_container_add (GTK_CONTAINER (row), priv->main_box);
    gtk_widget_show_all (GTK_WIDGET (row));

    G_OBJECT_CLASS (wb_timeline_row_parent_class)->constructed (object);
}

static void
wb_timeline_row_finalize (GObject *object)
{
    WbTimelineRow *self = WB_TIMELINE_ROW (object);
    WbTimelineRowPrivate *priv = wb_timeline_row_get_instance_private (self);

    g_array_free (priv->post_item->picuri_array, TRUE);
    g_free (priv->post_item->created_at);
    g_free (priv->post_item->idstr);
    g_free (priv->post_item->text);
    g_free (priv->post_item->source);
    g_free (priv->post_item->thumbnail_pic);
    g_free (priv->post_item->bmiddle_pic);
    g_free (priv->post_item->user->idstr);
    g_free (priv->post_item->user->name);
    g_free (priv->post_item->user->nickname);
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
wb_timeline_row_get_property (GObject *object,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
    WbTimelineRow *self = WB_TIMELINE_ROW (object);
    WbTimelineRowPrivate *priv = wb_timeline_row_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_POST_ITEM:
            g_value_set_pointer (value, priv->post_item);
            break;
        case PROP_RETWEET:
            g_value_set_boolean (value, priv->retweet);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
wb_timeline_row_set_property (GObject *object,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
    WbTimelineRow *self = WB_TIMELINE_ROW (object);
    WbTimelineRowPrivate *priv = wb_timeline_row_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_POST_ITEM:
            priv->post_item = g_value_get_pointer (value);
            break;
        case PROP_RETWEET:
            priv->retweet = g_value_get_boolean (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
wb_timeline_row_class_init (WbTimelineRowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructed = wb_timeline_row_constructed;
    gobject_class->finalize = wb_timeline_row_finalize;
    gobject_class->get_property = wb_timeline_row_get_property;
    gobject_class->set_property = wb_timeline_row_set_property;

    obj_properties[PROP_POST_ITEM] = g_param_spec_pointer ("post-item", "item",
                                                           "Post item for each row",
                                                           G_PARAM_READWRITE |
                                                           G_PARAM_CONSTRUCT_ONLY |
                                                           G_PARAM_STATIC_STRINGS);
    obj_properties[PROP_RETWEET] = g_param_spec_boolean ("retweet",
                                                         "Retweet",
                                                         "Retweeted post or not",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT);
    g_object_class_install_properties (gobject_class, N_PROPERTIES,
                                       obj_properties);
}

static void
wb_timeline_row_init (WbTimelineRow *self)
{
    WbTimelineRowPrivate *priv;

    priv = wb_timeline_row_get_instance_private (self);

    priv->profile_image = gtk_image_new_from_pixbuf (NULL);
    priv->post_image = gtk_image_new_from_pixbuf (NULL);

    priv->main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_start (priv->main_box, 12);
    gtk_widget_set_margin_end (priv->main_box, 12);
    gtk_widget_set_margin_top (priv->main_box, 6);
    gtk_widget_set_margin_bottom (priv->main_box, 6);
}

/**
 * wb_timeline_row_new:
 *
 * Create a new #WbTimelineRow.
 *
 * Returns: (transfer full): a newly created #WbTimelineRow
 */
WbTimelineRow *
wb_timeline_row_new (WbPostItem *post_item,
                     gboolean retweet)
{
    return g_object_new (WB_TYPE_TIMELINE_ROW,
                         "post-item", post_item,
                         "retweet", retweet,
                         NULL);
}
