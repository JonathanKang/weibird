/*
 *  Weibird - view and compose weibo
 *  copyright (c) 2019 jonathan kang <jonathankang@gnome.org>.
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

#include "wb-image-button.h"
#include "wb-multi-media-widget.h"
#include "wb-tweet-detail-page.h"
#include "wb-util.h"

enum
{
    PROP_0,
    PROP_POST_ITEM,
    N_PROPERTIES
};

struct _WbTweetDetailPage
{
		GtkBox parent_instance;
};

typedef struct
{
    WbPostItem *post_item;
} WbTweetDetailPagePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbTweetDetailPage, wb_tweet_detail_page, GTK_TYPE_BOX)

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
wb_tweet_detail_page_constructed (GObject *object)
{
    gchar *markup;
    GtkStyleContext *context;
    GtkWidget *hbox1;
    GtkWidget *hbox2;
    GtkWidget *vbox;
    GtkWidget *name_label;
    GtkWidget *pic_grid;
    GtkWidget *profile_image;
    GtkWidget *source_label;
    GtkWidget *text_label;
    GtkWidget *time_label;
    GtkWidget *likes_label;
    GtkWidget *comments_label;
    GtkWidget *reposts_label;
    WbImageButton *button;
    WbTweetDetailPage *page = WB_TWEET_DETAIL_PAGE (object);
    WbTweetDetailPagePrivate *priv = wb_tweet_detail_page_get_instance_private (page);

    hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

    /* Profile image (50px by 50px), name, source and time */
    button = wb_image_button_new (WB_MEDIA_TYPE_AVATAR,
                                  priv->post_item->user->profile_image_url,
                                  1, 50, 50);
    profile_image = GTK_WIDGET (button);
    gtk_widget_set_halign (profile_image, GTK_ALIGN_START);
    gtk_box_pack_start (GTK_BOX (hbox1), profile_image,
                        FALSE, FALSE, 0);

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

    if (g_strcmp0 (priv->post_item->source, "") != 0)
    {
        gtk_widget_set_valign (name_label, GTK_ALIGN_END);

        source_label = gtk_label_new (NULL);
        context = gtk_widget_get_style_context (source_label);
        gtk_style_context_add_class (context, "dim-label");
        gtk_label_set_markup (GTK_LABEL (source_label), priv->post_item->source);
        gtk_widget_set_halign (source_label, GTK_ALIGN_START);
        gtk_widget_set_valign (source_label, GTK_ALIGN_START);
        gtk_box_pack_end (GTK_BOX (vbox), source_label, TRUE, TRUE, 0);
    }

    time_label = gtk_label_new (priv->post_item->created_at);
    context = gtk_widget_get_style_context (time_label);
    gtk_style_context_add_class (context, "dim-label");
    gtk_widget_set_halign (time_label, GTK_ALIGN_END);
    gtk_box_pack_end (GTK_BOX (hbox1), time_label, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (page), hbox1, FALSE, FALSE, 0);

    /* Post content */
    text_label = gtk_label_new (priv->post_item->text);
    gtk_widget_set_halign (text_label, GTK_ALIGN_START);
    gtk_label_set_line_wrap (GTK_LABEL (text_label), TRUE);
    gtk_box_pack_start (GTK_BOX (page), text_label, FALSE, FALSE, 0);

    /* Post image */
    /* TODO: Add support for display the original picture individually */
    if (priv->post_item->picuri_array->len != 0)
    {
        pic_grid = wb_multi_media_widget_new (priv->post_item->picuri_array->len,
                                              priv->post_item->picuri_array);
        gtk_widget_set_halign (pic_grid, GTK_ALIGN_CENTER);
        gtk_box_pack_start (GTK_BOX (page), pic_grid, FALSE, FALSE, 0);
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

    gtk_box_pack_start (GTK_BOX (page), hbox2, FALSE, FALSE, 0);

    gtk_widget_show_all (GTK_WIDGET (page));

    G_OBJECT_CLASS (wb_tweet_detail_page_parent_class)->constructed (object);
}

static void
wb_tweet_detail_page_get_property (GObject *object,
                           guint prop_id,
                           GValue *value,
                           GParamSpec *pspec)
{
    WbTweetDetailPage *self = WB_TWEET_DETAIL_PAGE (object);
    WbTweetDetailPagePrivate *priv = wb_tweet_detail_page_get_instance_private (self);

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
wb_tweet_detail_page_set_property (GObject *object,
                       guint prop_id,
                       const GValue *value,
                       GParamSpec *pspec)
{
    WbTweetDetailPage *self = WB_TWEET_DETAIL_PAGE (object);
    WbTweetDetailPagePrivate *priv = wb_tweet_detail_page_get_instance_private (self);

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
wb_tweet_detail_page_class_init (WbTweetDetailPageClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructed = wb_tweet_detail_page_constructed;
    /* gobject_class->finalize = wb_tweet_detail_page_finalize; */
    gobject_class->get_property = wb_tweet_detail_page_get_property;
    gobject_class->set_property = wb_tweet_detail_page_set_property;

    obj_properties[PROP_POST_ITEM] = g_param_spec_pointer ("post-item", "item",
                                                           "Post item for each row",
                                                           G_PARAM_READWRITE |
                                                           G_PARAM_CONSTRUCT_ONLY |
                                                           G_PARAM_STATIC_STRINGS);
    g_object_class_install_properties (gobject_class, N_PROPERTIES,
                                       obj_properties);
}

static void
wb_tweet_detail_page_init (WbTweetDetailPage *self)
{
    gtk_orientable_set_orientation (GTK_ORIENTABLE (self),
                                    GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_margin_start (GTK_WIDGET (self), 12);
    gtk_widget_set_margin_end (GTK_WIDGET (self), 12);
    gtk_widget_set_margin_top (GTK_WIDGET (self), 12);
    gtk_widget_set_margin_bottom (GTK_WIDGET (self), 12);
}

WbTweetDetailPage *
wb_tweet_detail_page_new (WbPostItem *post_item)
{
    return g_object_new (WB_TYPE_TWEET_DETAIL_PAGE,
                         "post-item", post_item,
                         NULL);
}
