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

#include <glib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <rest/oauth2-proxy.h>

#include "wb-enums.h"
#include "wb-main-widget.h"
#include "wb-tweet-item.h"
#include "wb-tweet-row.h"
#include "wb-util.h"

enum
{
    LOADED,
    LAST_SIGNAL
};

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
    WbTweetItem *tweet_item;
    WbTweetItem *retweeted_item;
} WbTimelineListPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbTimelineList, wb_timeline_list, GTK_TYPE_BOX)

static guint signals[LAST_SIGNAL] = { 0 };

WbTweetItem *
wb_timeline_list_get_tweet_item (WbTimelineList *self)
{
    WbTimelineListPrivate *priv = wb_timeline_list_get_instance_private (self);

    g_return_val_if_fail (WB_TIMELINE_LIST (self), NULL);

    return priv->tweet_item;
}

WbTweetItem *
wb_timeline_list_get_retweeted_item (WbTimelineList *self)
{
    WbTimelineListPrivate *priv = wb_timeline_list_get_instance_private (self);

    g_return_val_if_fail (WB_TIMELINE_LIST (self), NULL);

    return priv->retweeted_item;
}

GtkListBox *
wb_timeline_list_get_listbox (WbTimelineList *self)
{
    WbTimelineListPrivate *priv = wb_timeline_list_get_instance_private (self);

    g_return_val_if_fail (WB_TIMELINE_LIST (self), NULL);

    return priv->timeline_list;
}

static void
parse_weibo_post (JsonArray *array,
                  guint index,
                  JsonNode *element_node,
                  gpointer user_data)
{
    JsonObject *object;
    WbTweetItem *tweet_item;
    WbTweetRow *row;
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
    tweet_item = wb_tweet_item_new (object);

    if (index == 0 && priv->batch_fetched == 0)
    {
        priv->last_id = tweet_item->id;
    }
    if (tweet_item->id < priv->last_id)
    {
        priv->last_id = tweet_item->id;
        priv->last_idstr = tweet_item->idstr;
    }

    /* Add retweeted item to the row */
    if (json_object_has_member (object, "retweeted_status"))
    {
        GtkWidget *retweeted_widget;
        JsonObject *retweeted_object;
        WbTweetItem *retweeted_item;

        retweeted_object = json_object_get_object_member (object, "retweeted_status");
        retweeted_item = wb_tweet_item_new (retweeted_object);
        /* TODO: Add WbRetweetedItem class? */
        retweeted_widget = GTK_WIDGET (wb_tweet_row_new (retweeted_item,
                                                         NULL, TRUE));

        row = wb_tweet_row_new (tweet_item, retweeted_item, FALSE);
        gtk_list_box_insert (GTK_LIST_BOX (priv->timeline_list),
                             GTK_WIDGET (row), -1);

        wb_tweet_row_insert_retweeted_item (row, retweeted_widget);

        g_object_unref (retweeted_item);
    }
    else
    {
        row = wb_tweet_row_new (tweet_item, NULL, FALSE);
        gtk_list_box_insert (GTK_LIST_BOX (priv->timeline_list),
                             GTK_WIDGET (row), -1);
    }

    g_object_unref (tweet_item);
}

static void
statuses_home_timeline_finished_cb (RestProxyCall *call,
                                    const GError *error,
                                    GObject *weak_object,
                                    gpointer user_data)
{
    const gchar *payload;
    goffset payload_length;
    GError *err = NULL;
    JsonNode *root_node;
    JsonParser *parser;
    WbTimelineList *self;
    WbTimelineListPrivate *priv;

    self = WB_TIMELINE_LIST (user_data);
    priv = wb_timeline_list_get_instance_private (self);

    if (error != NULL)
    {
        g_error ("Error calling Weibo API(2/statuses/home_timeline): %s",
                 error->message);
        return;
    }

    payload = rest_proxy_call_get_payload (call);
    payload_length = rest_proxy_call_get_payload_length (call);

    parser = json_parser_new ();
    if (!json_parser_load_from_data (parser, payload, payload_length, &err))
    {
        g_warning ("json_parser_load_from_data () failed: %s (%s, %d)",
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

        if (json_object_has_member (object, "statuses"))
        {
            JsonArray *array;

            array = json_object_get_array_member (object, "statuses");
            json_array_foreach_element (array, parse_weibo_post, self);

            priv->batch_fetched++;
        }
    }

    g_signal_emit (self, signals[LOADED], 0);

    g_object_unref (parser);
}

void
wb_timeline_list_get_home_timeline (WbTimelineList *self,
                                    gboolean loading_more)
{
    g_autofree gchar *access_token = NULL;
    g_autofree gchar *app_key = NULL;
    GError *error = NULL;
    RestProxy *proxy;
    RestProxyCall *call;
    WbTimelineListPrivate *priv;

    priv = wb_timeline_list_get_instance_private (self);

    access_token = wb_util_get_access_token ();
    app_key = wb_util_get_app_key ();

    proxy = oauth2_proxy_new_with_token (app_key, access_token,
                                         "https://api.weibo.com/oauth2/authorize",
                                         "https://api.weibo.com", FALSE);
    call = rest_proxy_new_call (proxy);
    rest_proxy_call_set_function (call, "2/statuses/home_timeline.json");
    rest_proxy_call_set_method (call, "GET");
    rest_proxy_call_add_param (call, "access_token", access_token);
    if (loading_more)
    {
        rest_proxy_call_add_param (call, "max_id", priv->last_idstr);
    }

    if (!rest_proxy_call_async (call, statuses_home_timeline_finished_cb,
                                NULL, self, &error))
    {
        g_error ("API(2/statues/home_timeline) call cancelled: %s",
                 error->message);
        g_error_free (error);
    }

    g_object_unref (call);
    g_object_unref (proxy);
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
row_activated_cb (GtkListBox *box,
                  GtkListBoxRow *row,
                  gpointer user_data)
{
    GtkWidget *toplevel;
    WbTimelineList *list;
    WbTimelineListPrivate *priv;

    list = WB_TIMELINE_LIST (user_data);
    priv = wb_timeline_list_get_instance_private (list);

    priv->tweet_item = wb_tweet_row_get_tweet_item (WB_TWEET_ROW (row));
    priv->retweeted_item = wb_tweet_row_get_retweeted_item (WB_TWEET_ROW (row));
    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (list));

    if (gtk_widget_is_toplevel (toplevel))
    {
        GAction *mode;
        GEnumClass *eclass;
        GEnumValue *evalue;

        mode = g_action_map_lookup_action (G_ACTION_MAP (toplevel), "view-mode");
        eclass = g_type_class_ref (WB_TYPE_MAIN_WIDGET_MODE);
        evalue = g_enum_get_value (eclass, WB_MAIN_WIDGET_MODE_DETAIL);

        g_action_activate (mode, g_variant_new_string (evalue->value_nick));

        g_type_class_unref (eclass);
    }
    else
    {
        g_debug ("Widget not in toplevel window, not switching toolbar mode");
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

    signals[LOADED] = g_signal_new ("loaded",
                                    G_TYPE_FROM_CLASS (klass),
                                    G_SIGNAL_RUN_LAST,
                                    0,
                                    NULL,
                                    NULL,
                                    NULL,
                                    G_TYPE_NONE,
                                    0);
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

    g_signal_connect (priv->timeline_list, "row-activated",
                      G_CALLBACK (row_activated_cb), self);
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
