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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <libsoup/soup.h>

#include "wb-avatar-widget.h"
#include "wb-image-button.h"
#include "wb-multi-media-widget.h"
#include "wb-name-button.h"
#include "wb-tweet-item.h"
#include "wb-tweet-row.h"
#include "wb-util.h"

enum
{
    PROP_0,
    PROP_TWEET_ITEM,
    PROP_RETWEET,
    N_PROPERTIES
};

struct _WbTweetRow
{
    /*< private >*/
    GtkListBoxRow parent_instance;
};

typedef struct
{
    gboolean retweet;
    GtkWidget *comment_button;
    GtkWidget *main_box;
    GtkWidget *retweet_box;
    GtkWidget *profile_image;
    GtkWidget *post_image;
    WbTweetItem *tweet_item;
    WbTweetItem *retweeted_item;
} WbTweetRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbTweetRow, wb_tweet_row, GTK_TYPE_LIST_BOX_ROW)

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

GtkWidget *
wb_tweet_row_get_comment_button (WbTweetRow *self)
{
    WbTweetRowPrivate *priv;

    g_return_val_if_fail (WB_TWEET_ROW (self), NULL);

    priv = wb_tweet_row_get_instance_private (self);

    return priv->comment_button;
}

WbTweetItem *
wb_tweet_row_get_tweet_item (WbTweetRow *self)
{
    WbTweetRowPrivate *priv = wb_tweet_row_get_instance_private (self);

    g_return_val_if_fail (WB_TWEET_ROW (self), NULL);

    return priv->tweet_item;
}

WbTweetItem *
wb_tweet_row_get_retweeted_item (WbTweetRow *self)
{
    WbTweetRowPrivate *priv = wb_tweet_row_get_instance_private (self);

    g_return_val_if_fail (WB_TWEET_ROW (self), NULL);

    return priv->retweeted_item;
}

void
wb_tweet_row_insert_retweeted_item (WbTweetRow *self,
                                    GtkWidget *retweeted_widget)
{
    GtkStyleContext *context;
    WbTweetRowPrivate *priv;

    priv = wb_tweet_row_get_instance_private (self);

    context = gtk_widget_get_style_context (retweeted_widget);
    gtk_style_context_add_class (context, "retweet");

    gtk_box_pack_end (GTK_BOX (priv->retweet_box), retweeted_widget,
                      TRUE, TRUE, 0);
}

static void
wb_tweet_row_constructed (GObject *object)
{
    gchar *created_at;
    gchar *markup;
    GtkStyleContext *context;
    GtkWidget *hbox1;
    GtkWidget *hbox2;
    GtkWidget *vbox;
    GtkWidget *source_label;
    GtkWidget *text_label;
    GtkWidget *time_label;
    WbAvatarWidget *avatar;
    WbMultiMediaWidget *pic_grid;
    WbNameButton *name_button;
    WbTweetRow *row = WB_TWEET_ROW (object);
    WbTweetRowPrivate *priv = wb_tweet_row_get_instance_private (row);

    hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start (GTK_BOX (priv->main_box), hbox1, FALSE, FALSE, 0);

    if (!priv->retweet)
    {
        /* Profile image (50px by 50px), name, source and time */
        avatar = wb_avatar_widget_new ();
        wb_avatar_widget_setup (avatar,
                                priv->tweet_item->user->profile_image_url);
        priv->profile_image = GTK_WIDGET (avatar);
        gtk_widget_set_halign (priv->profile_image, GTK_ALIGN_START);
        gtk_box_pack_start (GTK_BOX (hbox1), priv->profile_image,
                            FALSE, FALSE, 0);
    }

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (hbox1), vbox, FALSE, FALSE, 0);

    if (g_strcmp0 (priv->tweet_item->user->nickname, "") != 0)
    {
        name_button = wb_name_button_new ();
        wb_name_button_set_text (name_button, priv->tweet_item->user->nickname);
    }
    else
    {
        name_button = wb_name_button_new ();
        wb_name_button_set_text (name_button, priv->tweet_item->user->name);
    }
    gtk_widget_set_halign (GTK_WIDGET (name_button), GTK_ALIGN_START);
    gtk_widget_set_valign (GTK_WIDGET (name_button), GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (name_button), TRUE, TRUE, 0);

    if (!priv->retweet && g_strcmp0 (priv->tweet_item->source, "") != 0)
    {
        gchar *source;

        gtk_widget_set_valign (GTK_WIDGET (name_button), GTK_ALIGN_END);

        source = wb_util_format_source_string (priv->tweet_item->source);
        source_label = gtk_label_new (source);
        context = gtk_widget_get_style_context (source_label);
        gtk_style_context_add_class (context, "dim-label");
        gtk_widget_set_halign (source_label, GTK_ALIGN_START);
        gtk_widget_set_valign (source_label, GTK_ALIGN_START);
        gtk_box_pack_end (GTK_BOX (vbox), source_label, TRUE, TRUE, 0);

        g_free (source);
    }

    created_at = wb_util_format_time_string (priv->tweet_item->created_at);
    time_label = gtk_label_new (created_at);
    context = gtk_widget_get_style_context (time_label);
    gtk_style_context_add_class (context, "dim-label");
    gtk_widget_set_halign (time_label, GTK_ALIGN_END);
    gtk_box_pack_end (GTK_BOX (hbox1), time_label, FALSE, FALSE, 0);

    /* Post content */
    text_label = gtk_label_new (priv->tweet_item->text);
    gtk_widget_set_halign (text_label, GTK_ALIGN_START);
    gtk_label_set_xalign (GTK_LABEL (text_label), 0);
    gtk_label_set_line_wrap (GTK_LABEL (text_label), TRUE);
    gtk_box_pack_start (GTK_BOX (priv->main_box), text_label, FALSE, FALSE, 0);

    /* Post image */
    /* TODO: Add support for display the original picture individually */
    if (priv->tweet_item->picuri_array->len != 0)
    {
        pic_grid = wb_multi_media_widget_new ();
        wb_multi_media_widget_populate_images (pic_grid,
                                               priv->tweet_item->picuri_array);
        gtk_widget_set_halign (GTK_WIDGET (pic_grid), GTK_ALIGN_CENTER);
        gtk_box_pack_start (GTK_BOX (priv->main_box), GTK_WIDGET (pic_grid),
                            FALSE, FALSE, 0);
    }

    /* Retweeted post goes here, if there is one. */
    priv->retweet_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (priv->main_box), priv->retweet_box,
                        FALSE, FALSE, 0);

    /* Likes, comments and reposts count */
    if (!priv->retweet)
    {
        GtkStyleContext *context;
        GtkWidget *likes_button;
        GtkWidget *likes_label;
        GtkWidget *comments_label;
        GtkWidget *reposts_button;
        GtkWidget *reposts_label;

        likes_button = gtk_button_new ();
        context = gtk_widget_get_style_context (likes_button);
        gtk_style_context_add_class (context, "attitude-buttons");

        likes_label = gtk_label_new (NULL);
        markup = g_markup_printf_escaped ("<b>%d</b> Likes",
                                          priv->tweet_item->attitudes_count);
        gtk_label_set_markup (GTK_LABEL (likes_label), markup);
        gtk_container_add (GTK_CONTAINER (likes_button), likes_label);
        gtk_box_pack_start (GTK_BOX (hbox2), likes_button, TRUE, TRUE, 0);
        g_free (markup);

        priv->comment_button = gtk_button_new ();
        context = gtk_widget_get_style_context (priv->comment_button);
        gtk_style_context_add_class (context, "attitude-buttons");

        comments_label = gtk_label_new (NULL);
        markup = g_markup_printf_escaped ("<b>%d</b> Comments",
                                          priv->tweet_item->comments_count);
        gtk_label_set_markup (GTK_LABEL (comments_label), markup);
        gtk_container_add (GTK_CONTAINER (priv->comment_button), comments_label);
        gtk_box_pack_start (GTK_BOX (hbox2), priv->comment_button,
                            TRUE, TRUE, 0);
        g_free (markup);

        reposts_button = gtk_button_new ();
        context = gtk_widget_get_style_context (reposts_button);
        gtk_style_context_add_class (context, "attitude-buttons");

        reposts_label = gtk_label_new (NULL);
        markup = g_markup_printf_escaped ("<b>%d</b> Reposts",
                                          priv->tweet_item->reposts_count);
        gtk_label_set_markup (GTK_LABEL (reposts_label), markup);
        gtk_container_add (GTK_CONTAINER (reposts_button), reposts_label);
        gtk_box_pack_start (GTK_BOX (hbox2), reposts_button, TRUE, TRUE, 0);
        g_free (markup);

        gtk_box_pack_start (GTK_BOX (priv->main_box), hbox2, FALSE, FALSE, 0);
    }

    gtk_container_add (GTK_CONTAINER (row), priv->main_box);
    gtk_widget_show_all (GTK_WIDGET (row));

    g_free (created_at);

    G_OBJECT_CLASS (wb_tweet_row_parent_class)->constructed (object);
}

static void
wb_tweet_row_finalize (GObject *object)
{
    WbTweetRow *self = WB_TWEET_ROW (object);
    WbTweetRowPrivate *priv = wb_tweet_row_get_instance_private (self);

    g_object_unref (priv->tweet_item);

    G_OBJECT_CLASS (wb_tweet_row_parent_class)->finalize (object);
}

static void
wb_tweet_row_get_property (GObject *object,
                       guint prop_id,
                       GValue *value,
                       GParamSpec *pspec)
{
    WbTweetRow *self = WB_TWEET_ROW (object);
    WbTweetRowPrivate *priv = wb_tweet_row_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_TWEET_ITEM:
            g_value_set_object (value, priv->tweet_item);
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
wb_tweet_row_set_property (GObject *object,
                       guint prop_id,
                       const GValue *value,
                       GParamSpec *pspec)
{
    WbTweetRow *self = WB_TWEET_ROW (object);
    WbTweetRowPrivate *priv = wb_tweet_row_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_TWEET_ITEM:
            priv->tweet_item = g_value_dup_object (value);
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
wb_tweet_row_class_init (WbTweetRowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructed = wb_tweet_row_constructed;
    gobject_class->finalize = wb_tweet_row_finalize;
    gobject_class->get_property = wb_tweet_row_get_property;
    gobject_class->set_property = wb_tweet_row_set_property;

    obj_properties[PROP_TWEET_ITEM] = g_param_spec_object ("tweet-item",
                                                           "item",
                                                           "Tweet item for each row",
                                                           WB_TYPE_TWEET_ITEM,
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
wb_tweet_row_init (WbTweetRow *self)
{
    WbTweetRowPrivate *priv;

    priv = wb_tweet_row_get_instance_private (self);

    priv->profile_image = gtk_image_new_from_pixbuf (NULL);
    priv->post_image = gtk_image_new_from_pixbuf (NULL);

    priv->main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_start (priv->main_box, 12);
    gtk_widget_set_margin_end (priv->main_box, 12);
    gtk_widget_set_margin_top (priv->main_box, 6);
    gtk_widget_set_margin_bottom (priv->main_box, 6);
}

/**
 * wb_tweet_row_new:
 *
 * Create a new #WbTweetRow.
 *
 * Returns: (transfer full): a newly created #WbTweetRow
 */
WbTweetRow *
wb_tweet_row_new (WbTweetItem *tweet_item,
                  WbTweetItem *retweeted_item,
                  gboolean retweet)
{
    WbTweetRow *self;
    WbTweetRowPrivate *priv;

    self = g_object_new (WB_TYPE_TWEET_ROW,
                         "tweet-item", tweet_item,
                         "retweet", retweet,
                         NULL);
    priv = wb_tweet_row_get_instance_private (self);

    priv->retweeted_item = retweeted_item;

    return self;
}
