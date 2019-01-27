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

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <rest/oauth2-proxy.h>

#include "wb-timeline-list.h"
#include "wb-timeline-row.h"
#include "wb-util.h"

struct _WbTimelineList
{
    /*< private >*/
    GtkBox parent_instance;
};

typedef struct
{
    gint batch_fetched;
    gint64 last_id;
    gchar *last_idstr;
    GtkListBox *timeline_list;
    GtkWidget *timeline_scrolled;
} WbTimelineListPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbTimelineList, wb_timeline_list, GTK_TYPE_BOX)

static void
parse_weibo_post (JsonArray *array,
                  guint index,
                  JsonNode *element_node,
                  gpointer user_data)
{
    GtkWidget *retweeted_item;
    WbPostItem *post_item;
    WbUser *user;
    JsonObject *object;
    WbTimelineRow *row;
    WbTimelineList *self;
    WbTimelineListPrivate *priv;

    self = WB_TIMELINE_LIST (user_data);
    priv = wb_timeline_list_get_instance_private (self);

    /* Weibo API returns an extra item which can be found in the
     * previous batch of posts fetched before. In this case,
     * ingore the first post item. */
    if (index == 0 && priv->batch_fetched != 0)
    {
        return;
    }

    object = json_node_get_object (element_node);
    post_item = g_malloc0 (sizeof (WbPostItem));
    user = g_malloc0 (sizeof (WbUser));
    post_item->user = user;

    wb_util_parse_weibo_post (object, post_item);

    if (index == 0 && priv->batch_fetched == 0)
    {
        priv->last_id = post_item->id;
    }
    if (post_item->id < priv->last_id)
    {
        priv->last_id = post_item->id;
        priv->last_idstr = post_item->idstr;
    }

    row = wb_timeline_row_new (post_item);
    gtk_list_box_insert (GTK_LIST_BOX (priv->timeline_list),
                         GTK_WIDGET (row), -1);

    /* Add retweeted item to the row */
    if (json_object_has_member (object, "retweeted_status"))
    {
        WbPostItem *retweeted_post_item;
        WbUser *retweeted_user;
        JsonObject *retweet_object;

        retweeted_post_item = g_malloc0 (sizeof (WbPostItem));
        retweeted_user = g_malloc0 (sizeof (WbUser));
        retweeted_post_item->user = retweeted_user;

        retweet_object = json_object_get_object_member (object, "retweeted_status");
        wb_util_parse_weibo_post (retweet_object, retweeted_post_item);
        /* TODO: Add WbRetweetedItem class? */
        retweeted_item = GTK_WIDGET (wb_timeline_row_new (retweeted_post_item));

        wb_timeline_row_insert_retweeted_item (row, retweeted_item);
    }
}

void
wb_timeline_list_get_home_timeline (WbTimelineList *self,
                                    gboolean loading_more)
{
    const gchar *payload;
    GError *error = NULL;
    goffset payload_length;
    JsonNode *root_node;
    JsonParser *parser;
    RestProxy *proxy;
    RestProxyCall *call;
    WbTimelineListPrivate *priv;

    priv = wb_timeline_list_get_instance_private (self);

    proxy = oauth2_proxy_new_with_token ("1450991920",
                                         "2.005ugqwDURNMaB003aa34d9b828hsD",
                                         "https://api.weibo.com/oauth2/authorize",
                                         "https://api.weibo.com", FALSE);
    call = rest_proxy_new_call (proxy);
    rest_proxy_call_set_function (call, "2/statuses/home_timeline.json");
    rest_proxy_call_set_method (call, "GET");
    rest_proxy_call_add_param (call, "access_token",
                               "2.005ugqwDURNMaB003aa34d9b828hsD");
    if (loading_more)
    {
        rest_proxy_call_add_param (call, "max_id", priv->last_idstr);
    }

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
        g_warning ("json_parser_load_from_data () failed: %s (%s, %d)",
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

        if (json_object_has_member (object, "statuses"))
        {
            JsonArray *array;

            array = json_object_get_array_member (object, "statuses");
            json_array_foreach_element (array, parse_weibo_post, self);

            priv->batch_fetched++;
        }
    }

    g_object_unref (parser);
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
wb_timeline_list_edge_reached (GtkScrolledWindow *scrolled_window,
                               GtkPositionType pos,
                               gpointer user_data)
{
    WbTimelineList *list;

    list = WB_TIMELINE_LIST (user_data);

    if (pos == GTK_POS_BOTTOM)
    {
        wb_timeline_list_get_home_timeline (list, TRUE);
    }
}

static void
wb_timeline_list_class_init (WbTimelineListClass *klass)
{
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/com/jonathankang/Weibird/wb-timeline-list.ui");
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTimelineList, timeline_list);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbTimelineList, timeline_scrolled);
}

static void
wb_timeline_list_init (WbTimelineList *self)
{
    WbTimelineListPrivate *priv;

    gtk_widget_init_template (GTK_WIDGET (self));

    priv = wb_timeline_list_get_instance_private (self);

    priv->batch_fetched = 0;

    gtk_list_box_set_header_func (priv->timeline_list,
                                  (GtkListBoxUpdateHeaderFunc) listbox_update_header_func,
                                  NULL, NULL);

    g_signal_connect (priv->timeline_scrolled, "edge-reached",
                      G_CALLBACK (wb_timeline_list_edge_reached), self);
}

/**
 * wb_timeline_list_new:
 *
 * Create a new #WbTimelineList.
 *
 * Returns: (transfer full): a newly created #WbTimelineList
 */
WbTimelineList *
wb_timeline_list_new (void)
{
    return g_object_new (WB_TYPE_TIMELINE_LIST, NULL);
}
