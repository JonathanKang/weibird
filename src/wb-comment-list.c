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

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <rest/oauth2-proxy.h>

#include "wb-comment.h"
#include "wb-comment-list.h"
#include "wb-comment-row.h"
#include "wb-util.h"

struct _WbCommentList
{
		GtkListBox parent_instance;
};

typedef struct
{
    const gchar *tweet_id;
} WbCommentListPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbCommentList, wb_comment_list, GTK_TYPE_LIST_BOX)

static const gchar SETTINGS_SCHEMA[] = "com.jonathankang.Weibird";
static const gchar ACCESS_TOKEN[] = "access-token";

void
wb_comment_list_set_tweet_id (WbCommentList *self,
                              const gchar *tweet_id)
{
    WbCommentListPrivate *priv;

    priv = wb_comment_list_get_instance_private (self);

    priv->tweet_id = tweet_id;
}

static void
wb_comment_list_add_comment (WbCommentList *self,
                             const gchar *cid,
                             const gchar *comment)
{
    gchar *access_token;
    GError *error = NULL;
    GSettings *settings;
    RestProxy *proxy;
    RestProxyCall *call;
    WbCommentListPrivate *priv;

    priv = wb_comment_list_get_instance_private (self);

    settings = g_settings_new (SETTINGS_SCHEMA);
    access_token = g_settings_get_string (settings, ACCESS_TOKEN);

    proxy = oauth2_proxy_new_with_token (APP_KEY, access_token,
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
        const gchar *payload;
        gssize payload_length;
        GError *parser_error = NULL;
        JsonNode *root_node;
        JsonParser *parser;

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
            g_warning ("Cannot make call (comments/reply): %s", error->message);
            g_error_free (error);
        }

        g_object_unref (parser);
    }

    g_free (access_token);
    g_object_unref (settings);
}

static void
row_activated_cb (GtkListBox *box,
                  GtkListBoxRow *row,
                  gpointer user_data)
{
    const gchar *comment_str;
    gint result;
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *comment_entry;
    GtkWidget *toplevel;
    WbComment *comment;
    WbCommentList *self;

    self = WB_COMMENT_LIST (box);

    comment = wb_comment_row_get_comment (WB_COMMENT_ROW (row));

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (box));

    dialog = gtk_dialog_new_with_buttons ("Reply to Comment",
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
        comment_str = gtk_entry_get_text (GTK_ENTRY (comment_entry));
        wb_comment_list_add_comment (self, comment->idstr, comment_str);
    }

    gtk_widget_destroy (dialog);
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
wb_comment_list_class_init (WbCommentListClass *klass)
{
}

static void
wb_comment_list_init (WbCommentList *self)
{
    WbCommentListPrivate *priv;

    priv = wb_comment_list_get_instance_private (self);

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
