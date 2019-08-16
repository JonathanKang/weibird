/*
 *  Weibird - View and compose weibo
 *  Copyright (C) 2018 Jonathan Kang <jonathankang@gnome.org>.
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
#include <gmodule.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <rest/oauth2-proxy.h>

#include "wb-comment.h"
#include "wb-comment-list.h"
#include "wb-comment-row.h"
#include "wb-compose-window.h"
#include "wb-util.h"

struct _WbCommentList
{
		GtkListBox parent_instance;
};

typedef struct
{
    const gchar *tweet_id;
    gchar *current_cid;
    GHashTable *comments;
} WbCommentListPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbCommentList, wb_comment_list, GTK_TYPE_LIST_BOX)

void
wb_comment_list_set_tweet_id (WbCommentList *self,
                              const gchar *tweet_id)
{
    WbCommentListPrivate *priv;

    priv = wb_comment_list_get_instance_private (self);

    priv->tweet_id = tweet_id;
}

static void
parse_weibo_comments (gpointer data,
                      gpointer user_data)
{
    JsonObject *object;
    WbComment *comment;
    WbCommentRow *comment_row;
    WbCommentList *self = WB_COMMENT_LIST (user_data);

    object = json_node_get_object (data);
    comment = wb_comment_new (object);
    comment_row = wb_comment_row_new (comment);

    wb_comment_list_insert_comment_widget (self, comment_row);

    g_object_unref (comment);
}

static void
comments_show_finished_cb (RestProxyCall *call,
                           const GError *error,
                           GObject *weak_object,
                           gpointer user_data)
{
    const gchar *payload;
    goffset payload_length;
    GError *err = NULL;
    JsonParser *parser;
    JsonNode *root_node;
    WbCommentList *self;

    if (error != NULL)
    {
        g_error ("Error calling Weibo API(2/comments/show): %s",
                 error->message);
        return;
    }

    self = WB_COMMENT_LIST (user_data);

    payload = rest_proxy_call_get_payload (call);
    payload_length = rest_proxy_call_get_payload_length (call);

    parser = json_parser_new ();
    if (!json_parser_load_from_data (parser, payload, payload_length, &err))
    {
        g_warning ("json_parser_load_data () failed: %s (%s, %d)",
                   err->message,
                   g_quark_to_string (err->domain),
                   err->code);
        g_error_free (err);
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

void
wb_comment_list_load_comments (WbCommentList *self,
                               const gchar *idstr)
{
    g_autofree gchar *access_token = NULL;
    g_autofree gchar *app_key = NULL;
    g_autoptr (GError) error = NULL;
    RestProxy *proxy;
    RestProxyCall *call;

    access_token = wb_util_get_access_token ();
    app_key = wb_util_get_app_key ();

    proxy = oauth2_proxy_new_with_token (app_key, access_token,
                                         "https://api.weibo.com/oauth2/authorize",
                                         "https://api.weibo.com", FALSE);
    call = rest_proxy_new_call (proxy);
    rest_proxy_call_set_function (call, "2/comments/show.json");
    rest_proxy_call_set_method (call, "GET");
    rest_proxy_call_add_param (call, "access_token", access_token);
    rest_proxy_call_add_param (call, "id", idstr);

    rest_proxy_call_async (call, comments_show_finished_cb,
                           NULL, self, &error);

    g_object_unref (call);
    g_object_unref (proxy);
}

void
wb_comment_list_insert_comment_widget (WbCommentList *self,
                                       WbCommentRow *comment_widget)
{
    WbComment *comment;
    WbCommentListPrivate *priv;

    priv = wb_comment_list_get_instance_private (self);

    comment = wb_comment_row_get_comment (comment_widget);
    g_hash_table_insert (priv->comments, &comment->id, comment_widget);

    if (comment->reply_comment)
    {
        /* This is a comment which replies to another one. */
        WbCommentRow *root_comment;

        root_comment = g_hash_table_lookup (priv->comments, &comment->rootid);

        /* The root comment is somehow not found in the comment list. It
         * can be caused by the broken Weibo API: Not all the comments
         * will be available. */
        if (root_comment == NULL)
        {
            return;
        }

        wb_comment_row_insert_reply (WB_COMMENT_ROW (root_comment), comment);
    }
    else
    {
        /* This is not a comment which replies to any other ones,
         * insert it to the list box directly. */
        gtk_list_box_prepend (GTK_LIST_BOX (self), GTK_WIDGET (comment_widget));
    }
}

static void
wb_comment_list_add_comment (WbCommentList *self,
                             const gchar *cid,
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
    WbCommentListPrivate *priv;

    priv = wb_comment_list_get_instance_private (self);

    access_token = wb_util_get_access_token ();
    app_key = wb_util_get_app_key ();

    proxy = oauth2_proxy_new_with_token (app_key, access_token,
                                         "https://api.weibo.com/oauth2/authorize",
                                         "https://api.weibo.com", FALSE);
    call = rest_proxy_new_call (proxy);
    rest_proxy_call_set_function (call, "2/comments/reply.json");
    rest_proxy_call_set_method (call, "POST");
    rest_proxy_call_add_param (call, "access_token", access_token);
    rest_proxy_call_add_param (call, "id", priv->tweet_id);
    rest_proxy_call_add_param (call, "cid", cid);
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
            g_warning ("Error calling Weibo API(2/comments/reply): %s",
                       error->message);
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

            wb_comment_list_insert_comment_widget (WB_COMMENT_LIST (self),
                                                   comment_row);

            g_object_unref (comment);
        }
    }

    g_object_unref (call);
    g_object_unref (parser);
    g_object_unref (proxy);
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
            const char *comment;
            GtkWidget *compose_entry;
            WbCommentList *self;
            WbCommentListPrivate *priv;

            self = WB_COMMENT_LIST (user_data);
            priv = wb_comment_list_get_instance_private (self);

            compose_entry = wb_compose_window_get_compose_entry (WB_COMPOSE_WINDOW (dialog));
            comment = gtk_entry_get_text (GTK_ENTRY (compose_entry));
            wb_comment_list_add_comment (self, priv->current_cid, comment);

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
row_activated_cb (GtkListBox *box,
                  GtkListBoxRow *row,
                  gpointer user_data)
{
    GtkWidget *toplevel;
    WbComment *comment;
    WbComposeWindow *compose_window;
    WbCommentList *self;
    WbCommentListPrivate *priv;

    self = WB_COMMENT_LIST (box);
    priv = wb_comment_list_get_instance_private (self);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (box));
    comment = wb_comment_row_get_comment (WB_COMMENT_ROW (row));
    /* Remember which comment we are commenting on */
    priv->current_cid = g_strdup (comment->idstr);

    compose_window = wb_compose_window_new (GTK_WINDOW (toplevel));
    g_signal_connect (compose_window, "response",
                      G_CALLBACK (compose_dialog_response_cb), self);

    gtk_widget_show_all (GTK_WIDGET (compose_window));
}

static void
listbox_update_header_func (GtkListBoxRow *row,
                            GtkListBoxRow *before,
                            gpointer user_data)
{
    GtkWidget *header;

    if (before == NULL)
    {
        gtk_list_box_row_set_header (row, NULL);
        return;
    }

    header = gtk_list_box_row_get_header (row);
    if (header == NULL)
    {
        header = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_show (header);
        gtk_list_box_row_set_header (row, header);
    }
}

static void
wb_comment_list_finalize (GObject *object)
{
    WbCommentList *self;
    WbCommentListPrivate *priv;

    self = WB_COMMENT_LIST (object);
    priv = wb_comment_list_get_instance_private (self);

    g_free (priv->current_cid);

    g_hash_table_destroy (priv->comments);

    G_OBJECT_CLASS (wb_comment_list_parent_class)->finalize (object);
}

static void
wb_comment_list_class_init (WbCommentListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = wb_comment_list_finalize;
}

static void
wb_comment_list_init (WbCommentList *self)
{
    WbCommentListPrivate *priv;

    priv = wb_comment_list_get_instance_private (self);

    priv->comments = g_hash_table_new (g_int64_hash, g_int64_equal);
    priv->current_cid = NULL;
    priv->tweet_id = NULL;

    gtk_list_box_set_header_func (GTK_LIST_BOX (self),
                                  (GtkListBoxUpdateHeaderFunc) listbox_update_header_func,
                                  NULL, NULL);
    gtk_list_box_set_selection_mode (GTK_LIST_BOX (self), GTK_SELECTION_NONE);

    g_signal_connect (self, "row-activated",
                      G_CALLBACK (row_activated_cb), NULL);
}

/**
 * wb_comment_list_new:
 *
 * Create a new #WbCommentList.
 *
 * Returns: (transfer full): a newly created #WbCommentList
 */
WbCommentList *
wb_comment_list_new (void)
{
    return g_object_new (WB_TYPE_COMMENT_LIST, NULL);
}
