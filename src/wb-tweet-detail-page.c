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
#include <json-glib/json-glib.h>
#include <rest/oauth2-proxy.h>

#include "wb-comment.h"
#include "wb-comment-row.h"
#include "wb-image-button.h"
#include "wb-multi-media-widget.h"
#include "wb-tweet-detail-page.h"
#include "wb-tweet-row.h"
#include "wb-util.h"

enum
{
    PROP_0,
    PROP_TWEET_ITEM,
    PROP_RETWEETED_ITEM,
    N_PROPERTIES
};

struct _WbTweetDetailPage
{
		GtkScrolledWindow parent_instance;
};

typedef struct
{
    GtkWidget *main_box;
    WbTweetItem *tweet_item;
    WbTweetItem *retweeted_item;
} WbTweetDetailPagePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbTweetDetailPage, wb_tweet_detail_page, GTK_TYPE_SCROLLED_WINDOW)

static const gchar SETTINGS_SCHEMA[] = "com.jonathankang.Weibird";
static const gchar ACCESS_TOKEN[] = "access-token";
static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
parse_weibo_comments (JsonArray *array,
                      guint index,
                      JsonNode *element_node,
                      gpointer user_data)
{
    JsonObject *object;
    WbComment *comment;
    WbCommentRow *comment_row;
    WbTweetDetailPage *self;
    WbTweetDetailPagePrivate *priv;

    self = WB_TWEET_DETAIL_PAGE (user_data);
    priv = wb_tweet_detail_page_get_instance_private (self);

    object = json_node_get_object (element_node);
    comment = wb_comment_new (object);
    comment_row = wb_comment_row_new (comment);

    gtk_box_pack_start (GTK_BOX (priv->main_box), GTK_WIDGET (comment_row),
                        FALSE, FALSE, 0);

    g_object_unref (comment);
}

static void
fetch_comments (WbTweetDetailPage *self)
{
    const gchar *payload;
    gchar *access_token;
    goffset payload_length;
    GError *error = NULL;
    GSettings *settings;
    JsonNode *root_node;
    JsonParser *parser;
    RestProxy *proxy;
    RestProxyCall *call;
    WbTweetDetailPagePrivate *priv;

    priv = wb_tweet_detail_page_get_instance_private (self);

    settings = g_settings_new (SETTINGS_SCHEMA);
    access_token = g_settings_get_string (settings, ACCESS_TOKEN);

    proxy = oauth2_proxy_new_with_token (APP_KEY, access_token,
                                         "https://api.weibo.com/oauth2/authorize",
                                         "https://api.weibo.com", FALSE);
    call = rest_proxy_new_call (proxy);
    rest_proxy_call_set_function (call, "2/comments/show.json");
    rest_proxy_call_set_method (call, "GET");
    rest_proxy_call_add_param (call, "access_token", access_token);
    rest_proxy_call_add_param (call, "id", priv->tweet_item->idstr);

    rest_proxy_call_sync (call, &error);
    if (error != NULL)
    {
        g_error ("Cannot make call: %s", error->message);
        g_error_free (error);
    }

    payload = rest_proxy_call_get_payload (call);
    payload_length = rest_proxy_call_get_payload_length (call);

    parser = json_parser_new ();
    if (!json_parser_load_from_data (parser, payload, payload_length, &error))
    {
        g_warning ("json_parser_load_data () failed: %s (%s, %d)",
                   error->message,
                   g_quark_to_string (error->domain),
                   error->code);
        g_error_free (error);
    }

    root_node = json_parser_get_root (parser);
    if (JSON_NODE_HOLDS_OBJECT (root_node))
    {
        JsonObject *object;

        object = json_node_get_object (root_node);

        if (json_object_has_member (object, "comments"))
        {
            JsonArray *array;

            array = json_object_get_array_member (object, "comments");
            json_array_foreach_element (array, parse_weibo_comments, self);
        }
    }

    g_free (access_token);
    g_object_unref (parser);
    g_object_unref (settings);
}

static void
wb_tweet_detail_page_constructed (GObject *object)
{
    WbTweetRow *row;
    WbTweetDetailPage *self;
    WbTweetDetailPagePrivate *priv;

    self = WB_TWEET_DETAIL_PAGE (object);
    priv = wb_tweet_detail_page_get_instance_private (self);

    if (priv->retweeted_item != NULL)
    {
        WbTweetRow *retweeted_row;

        retweeted_row = wb_tweet_row_new (priv->retweeted_item, NULL, TRUE);

        row = wb_tweet_row_new (priv->tweet_item, priv->retweeted_item, FALSE);
        gtk_box_pack_start (GTK_BOX (priv->main_box), GTK_WIDGET (row),
                            FALSE, FALSE, 0);

        wb_tweet_row_insert_retweeted_item (row, GTK_WIDGET (retweeted_row));
    }
    else
    {
        row = wb_tweet_row_new (priv->tweet_item, NULL, FALSE);
        gtk_box_pack_start (GTK_BOX (priv->main_box), GTK_WIDGET (row),
                            FALSE, FALSE, 0);
    }

    g_idle_add (G_SOURCE_FUNC (fetch_comments), self);

    gtk_widget_show_all (GTK_WIDGET (self));

    G_OBJECT_CLASS (wb_tweet_detail_page_parent_class)->constructed (object);
}
static void
wb_tweet_detail_page_finalize (GObject *object)
{
    WbTweetDetailPage *self;
    WbTweetDetailPagePrivate *priv;

    self = WB_TWEET_DETAIL_PAGE (object);
    priv = wb_tweet_detail_page_get_instance_private (self);

    g_object_unref (priv->tweet_item);
    if (priv->retweeted_item != NULL)
    {
        g_object_unref (priv->retweeted_item);
    }

    G_OBJECT_CLASS (wb_tweet_detail_page_parent_class)->finalize (object);
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
        case PROP_TWEET_ITEM:
            g_value_set_object (value, priv->tweet_item);
            break;
        case PROP_RETWEETED_ITEM:
            g_value_set_object (value, priv->retweeted_item);
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
        case PROP_TWEET_ITEM:
            priv->tweet_item = g_value_dup_object (value);
            break;
        case PROP_RETWEETED_ITEM:
            priv->retweeted_item = g_value_dup_object (value);
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
    gobject_class->finalize = wb_tweet_detail_page_finalize;
    gobject_class->get_property = wb_tweet_detail_page_get_property;
    gobject_class->set_property = wb_tweet_detail_page_set_property;

    obj_properties[PROP_TWEET_ITEM] = g_param_spec_object ("tweet-item",
                                                           "item",
                                                           "Post item for each row",
                                                           WB_TYPE_TWEET_ITEM,
                                                           G_PARAM_READWRITE |
                                                           G_PARAM_CONSTRUCT_ONLY |
                                                           G_PARAM_STATIC_STRINGS);
    obj_properties[PROP_RETWEETED_ITEM] = g_param_spec_object ("retweeted-item",
                                                               "item",
                                                               "Retweeted item for retweet row",
                                                               WB_TYPE_TWEET_ITEM,
                                                               G_PARAM_READWRITE |
                                                               G_PARAM_CONSTRUCT_ONLY |
                                                               G_PARAM_STATIC_STRINGS);
    g_object_class_install_properties (gobject_class, N_PROPERTIES,
                                       obj_properties);
}

static void
wb_tweet_detail_page_init (WbTweetDetailPage *self)
{
    WbTweetDetailPagePrivate *priv;

    priv = wb_tweet_detail_page_get_instance_private (self);

    priv->main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add (GTK_CONTAINER (self), priv->main_box);
}

WbTweetDetailPage *
wb_tweet_detail_page_new (WbTweetItem *tweet_item,
                          WbTweetItem *retweeted_item)
{
    return g_object_new (WB_TYPE_TWEET_DETAIL_PAGE,
                         "tweet-item", tweet_item,
                         "retweeted-item", retweeted_item,
                         NULL);
}
