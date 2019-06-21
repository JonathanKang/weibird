/*
 *  Weibird - View and compose weibo
 *  Copyright (C) 2019 Jonathan Kang <jonathankang@gnome.org>.
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

#include <glib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <rest/oauth2-proxy.h>

#include "wb-avatar-widget.h"
#include "wb-comment.h"
#include "wb-comment-list.h"
#include "wb-comment-row.h"
#include "wb-compose-window.h"
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
    GtkWidget *avatar_widget;
    GtkWidget *listbox;
    GtkWidget *main_box;
    GtkWidget *buttons_box;
    GtkWidget *retweet_box;
    GtkWidget *name_label;
    GtkWidget *source_label;
    GtkWidget *time_label;
    GtkWidget *content_label;
    GtkWidget *likes_label;
    GtkWidget *comments_label;
    GtkWidget *reposts_label;
    WbMultiMediaWidget *mm_widget;
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

    g_object_unref (call);
    g_object_unref (parser);
    g_object_unref (proxy);
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
compose_dialog_response_cb (GtkDialog *dialog,
                            GtkResponseType response_type,
                            gpointer user_data)
{
    switch (response_type)
    {
        case GTK_RESPONSE_OK:
        {
            const gchar *comment;
            GtkWidget *compose_entry;
            WbTweetDetailPage *self;

            self = WB_TWEET_DETAIL_PAGE (user_data);

            compose_entry = wb_compose_window_get_compose_entry (WB_COMPOSE_WINDOW (dialog));
            comment = gtk_entry_get_text (GTK_ENTRY (compose_entry));
            wb_tweet_detail_page_add_comment (self, comment);

            gtk_widget_destroy (GTK_WIDGET (dialog));

            break;
        }
        case GTK_RESPONSE_CANCEL:
            gtk_widget_destroy (GTK_WIDGET (dialog));
            break;
        default:
            g_assert_not_reached ();
    }
}

static void
comment_button_clicked_cb (GtkButton *button,
                           gpointer user_data)
{
    GtkWidget *toplevel;
    WbComposeWindow *compose_window;
    WbTweetDetailPage *self;

    self = WB_TWEET_DETAIL_PAGE (user_data);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (button));

    compose_window = wb_compose_window_new (GTK_WINDOW (toplevel));
    g_signal_connect (compose_window, "response",
                      G_CALLBACK (compose_dialog_response_cb), self);

    gtk_widget_show_all (GTK_WIDGET (compose_window));
}

static void
wb_tweet_detail_page_constructed (GObject *object)
{
    gchar *created_at;
    gchar *markup;
    WbTweetDetailPage *self;
    WbTweetDetailPagePrivate *priv;

    self = WB_TWEET_DETAIL_PAGE (object);
    priv = wb_tweet_detail_page_get_instance_private (self);

    wb_avatar_widget_setup (WB_AVATAR_WIDGET (priv->avatar_widget),
                            priv->tweet_item->user->profile_image_url);

    if (g_strcmp0 (priv->tweet_item->user->nickname, "") != 0)
    {
        gtk_label_set_text (GTK_LABEL (priv->name_label),
                            priv->tweet_item->user->nickname);
    }
    else
    {
        gtk_label_set_text (GTK_LABEL (priv->name_label),
                            priv->tweet_item->user->name);
    }

    if (g_strcmp0 (priv->tweet_item->source, "") != 0)
    {
        gchar *source;

        source = wb_util_format_source_string (priv->tweet_item->source);
        gtk_label_set_text (GTK_LABEL (priv->source_label), source);

        g_free (source);
    }

    created_at = wb_util_format_time_string (priv->tweet_item->created_at);
    gtk_label_set_text (GTK_LABEL (priv->time_label), created_at);

    gtk_label_set_text (GTK_LABEL (priv->content_label), priv->tweet_item->text);

    /* Post image(s) */
    if (priv->tweet_item->picuri_array->len != 0)
    {
        wb_multi_media_widget_populate_images (priv->mm_widget,
                                               priv->tweet_item->picuri_array);
        gtk_widget_show_all (GTK_WIDGET (priv->mm_widget));
    }

    /* Likes, comments and reposts buttons. */
    markup = g_markup_printf_escaped ("<b>%d</b> Likes",
                                      priv->tweet_item->attitudes_count);
    gtk_label_set_markup (GTK_LABEL (priv->likes_label), markup);
    g_free (markup);

    markup = g_markup_printf_escaped ("<b>%d</b> Comments",
                                      priv->tweet_item->comments_count);
    gtk_label_set_markup (GTK_LABEL (priv->comments_label), markup);
    g_free (markup);

    markup = g_markup_printf_escaped ("<b>%d</b> Reposts",
                                      priv->tweet_item->reposts_count);
    gtk_label_set_markup (GTK_LABEL (priv->reposts_label), markup);
    g_free (markup);

    /* Retweeted content */
    if (priv->retweeted_item != NULL)
    {
        WbTweetRow *retweeted_row;

        retweeted_row = wb_tweet_row_new (priv->retweeted_item, NULL, TRUE);
        gtk_container_add (GTK_CONTAINER (priv->retweet_box),
                           GTK_WIDGET (retweeted_row));
    }
    else
    {
        gtk_widget_set_no_show_all (priv->retweet_box, TRUE);
    }

    g_idle_add (G_SOURCE_FUNC (fetch_comments), self);

    /* Pass tweet id down to WbCommentList */
    wb_comment_list_set_tweet_id (WB_COMMENT_LIST (priv->listbox),
                                  priv->tweet_item->idstr);

    gtk_widget_show_all (GTK_WIDGET (self));

    g_free (created_at);

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
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

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

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/com/jonathankang/Weibird/wb-tweet-detail-page.ui");
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  avatar_widget);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  main_box);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  buttons_box);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  retweet_box);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  name_label);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  source_label);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  time_label);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  content_label);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  likes_label);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  comments_label);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  reposts_label);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTweetDetailPage,
                                                  mm_widget);
    gtk_widget_class_bind_template_callback (widget_class,
                                             comment_button_clicked_cb);
}

static void
wb_tweet_detail_page_init (WbTweetDetailPage *self)
{
    WbTweetDetailPagePrivate *priv;

    priv = wb_tweet_detail_page_get_instance_private (self);

    gtk_widget_init_template (GTK_WIDGET (self));

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
