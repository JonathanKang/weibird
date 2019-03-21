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

#include <glib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <rest/oauth2-proxy.h>

#include "wb-comment.h"
#include "wb-comment-list.h"
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
    GtkWidget *listbox;
    GtkWidget *main_box;
    WbTweetItem *tweet_item;
    WbTweetItem *retweeted_item;
} WbTweetDetailPagePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbTweetDetailPage, wb_tweet_detail_page, GTK_TYPE_SCROLLED_WINDOW)

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
parse_weibo_comments (gpointer data,
                      gpointer user_data)
{
    JsonObject *object;
    WbComment *comment;
    WbCommentRow *comment_row;
    WbTweetDetailPage *self;
    WbTweetDetailPagePrivate *priv;

    self = WB_TWEET_DETAIL_PAGE (user_data);
    priv = wb_tweet_detail_page_get_instance_private (self);

    object = json_node_get_object (data);
    comment = wb_comment_new (object);
    comment_row = wb_comment_row_new (comment);

    wb_comment_list_insert_comment_widget (WB_COMMENT_LIST (priv->listbox),
                                           comment_row);

    g_object_unref (comment);
}

static void
fetch_comments (WbTweetDetailPage *self)
{
    const gchar *payload;
    g_autofree gchar *access_token = NULL;
    g_autofree gchar *app_key = NULL;
    goffset payload_length;
    GError *error = NULL;
    JsonNode *root_node;
    JsonParser *parser;
    RestProxy *proxy;
    RestProxyCall *call;
    WbTweetDetailPagePrivate *priv;

    priv = wb_tweet_detail_page_get_instance_private (self);

    access_token = wb_util_get_access_token ();
    app_key = wb_util_get_app_key ();

    proxy = oauth2_proxy_new_with_token (app_key, access_token,
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
            GList *elements;
            JsonArray *array;

            array = json_object_get_array_member (object, "comments");
            elements = json_array_get_elements (array);

            /* All the comments fetched using Weibo API are sorted by descending
             * time (new to old). While we need to insert older comments first,
             * so that when we insert newer comments, we can find which comment
             * it replies to, if it replies to one.
             * The foreach function of JsonArray doesn't iterate from the last
             * to the first element. So we need get a list of elements in the array
             * and reverse it. */
            elements = g_list_reverse (elements);
            g_list_foreach (elements, parse_weibo_comments, self);

            g_list_free (elements);
        }
    }

    g_object_unref (parser);
}

static void
wb_tweet_detail_page_add_comment (WbTweetDetailPage *self,
                                  const gchar *comment)
{
    const gchar *payload;
    g_autofree gchar *access_token = NULL;
    g_autofree gchar *app_key = NULL;
    gssize payload_length;
    GError *error = NULL;
    JsonNode *root_node;
    JsonParser *parser;
    RestProxy *proxy;
    RestProxyCall *call;
    WbTweetDetailPagePrivate *priv;

    priv = wb_tweet_detail_page_get_instance_private (self);

    access_token = wb_util_get_access_token ();
    app_key = wb_util_get_app_key ();

    proxy = oauth2_proxy_new_with_token (app_key, access_token,
                                         "https://api.weibo.com/oauth2/authorize",
                                         "https://api.weibo.com", FALSE);
    call = rest_proxy_new_call (proxy);
    rest_proxy_call_set_function (call, "2/comments/create.json");
    rest_proxy_call_set_method (call, "POST");
    rest_proxy_call_add_param (call, "access_token", access_token);
    rest_proxy_call_add_param (call, "id", priv->tweet_item->idstr);
    rest_proxy_call_add_param (call, "comment", comment);

    rest_proxy_call_sync (call, &error);
    if (error != NULL)
    {
        GError *parser_error = NULL;

        payload = rest_proxy_call_get_payload (call);
        payload_length = rest_proxy_call_get_payload_length (call);

        parser = json_parser_new ();
        if (!json_parser_load_from_data (parser, payload,
                                         payload_length, &parser_error))
        {
            g_warning ("Failed to parse comment data: %s (%s, %d)",
                       parser_error->message,
                       g_quark_to_string (parser_error->domain),
                       parser_error->code);
            g_error_free (parser_error);
        }

        root_node = json_parser_get_root (parser);
        if (JSON_NODE_HOLDS_OBJECT (root_node))
        {
            JsonObject *object;

            object = json_node_get_object (root_node);

            g_warning ("Failed to send request: %s (%s, %s)",
                       json_object_get_string_member (object, "request"),
                       json_object_get_string_member (object, "error_code"),
                       json_object_get_string_member (object, "error"));
        }
        else
        {
            g_warning ("Cannot make call (comments/create): %s", error->message);
            g_error_free (error);
        }
    }
    else
    {
        payload = rest_proxy_call_get_payload (call);
        payload_length = rest_proxy_call_get_payload_length (call);

        parser = json_parser_new ();
        if (!json_parser_load_from_data (parser, payload,
                                         payload_length, &error))
        {
            g_warning ("Failed to parse comment data: %s (%s, %d)",
                       error->message,
                       g_quark_to_string (error->domain),
                       error->code);
            g_error_free (error);
        }

        root_node = json_parser_get_root (parser);
        if (JSON_NODE_HOLDS_OBJECT (root_node))
        {
            JsonObject *object;
            WbComment *comment;
            WbCommentRow *comment_row;

            object = json_node_get_object (root_node);

            /* Parse the data and insert a comment row */
            comment = wb_comment_new (object);
            comment_row = wb_comment_row_new (comment);

            wb_comment_list_insert_comment_widget (WB_COMMENT_LIST (priv->listbox),
                                                   comment_row);

            g_object_unref (comment);
        }
    }

    g_object_unref (parser);
}

static void
comment_button_clicked_cb (GtkButton *button,
                           gpointer user_data)
{
    const gchar *comment;
    gint result;
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *comment_entry;
    GtkWidget *toplevel;
    WbTweetDetailPage *self;

    self = WB_TWEET_DETAIL_PAGE (user_data);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (button));

    dialog = gtk_dialog_new_with_buttons ("Compose Comment",
                                          GTK_WINDOW (toplevel),
                                          GTK_DIALOG_MODAL |
                                          GTK_DIALOG_DESTROY_WITH_PARENT |
                                          GTK_DIALOG_USE_HEADER_BAR,
                                          "Send",
                                          GTK_RESPONSE_OK,
                                          "Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          NULL);

    comment_entry = gtk_entry_new ();
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    gtk_container_add (GTK_CONTAINER (content_area), comment_entry);
    gtk_widget_show (comment_entry);

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    if (result == GTK_RESPONSE_OK)
    {
        comment = gtk_entry_get_text (GTK_ENTRY (comment_entry));
        wb_tweet_detail_page_add_comment (self, comment);
    }

    gtk_widget_destroy (dialog);
}

static void
wb_tweet_detail_page_constructed (GObject *object)
{
    GtkWidget *comment_button;
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

    /* Pass tweet id down to WbCommentList */
    wb_comment_list_set_tweet_id (WB_COMMENT_LIST (priv->listbox),
                                  priv->tweet_item->idstr);

    gtk_widget_show_all (GTK_WIDGET (self));

    comment_button = wb_tweet_row_get_comment_button (row);
    g_signal_connect (comment_button, "clicked",
                      G_CALLBACK (comment_button_clicked_cb), self);

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
    gtk_widget_set_valign (GTK_WIDGET (priv->main_box), GTK_ALIGN_START);
    gtk_container_add (GTK_CONTAINER (self), priv->main_box);

    priv->listbox = GTK_WIDGET (wb_comment_list_new ());
    gtk_box_pack_end (GTK_BOX (priv->main_box), priv->listbox,
                      FALSE, FALSE, 0);
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
