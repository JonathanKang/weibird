/*
 *  gnome weibo - view and compose weibo
 *  copyright (c) 2018 jonathan kang <jonathankang@gnome.org>.
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

#include "gw-timeline-list.h"
#include "gw-timeline-row.h"

struct _GwTimelineList
{
    /*< private >*/
    GtkBox parent_instance;
};

typedef struct
{
    GtkListBox *timeline_list;
} GwTimelineListPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GwTimelineList, gw_timeline_list, GTK_TYPE_BOX)

static void
parse_weibo_post (JsonArray *array,
                  guint index,
                  JsonNode *element_node,
                  gpointer user_data)
{
    GtkWidget *row;
    GwPostItem *post_item;
    GwUser *user;
    JsonObject *object;
    JsonObject *user_object;
    GwTimelineList *self;
    GwTimelineListPrivate *priv;

    self = GW_TIMELINE_LIST (user_data);
    priv = gw_timeline_list_get_instance_private (self);

    object = json_node_get_object (element_node);
    post_item = g_malloc0 (sizeof (GwPostItem));
    user = g_malloc0 (sizeof (GwUser));

    post_item->created_at = g_strdup (json_object_get_string_member (object, "created_at"));
    post_item->id = json_object_get_int_member (object, "id");
    post_item->mid = json_object_get_int_member (object, "mid");
    post_item->idstr = g_strdup (json_object_get_string_member (object, "idstr"));
    post_item->text = g_strdup (json_object_get_string_member (object, "text"));
    post_item->source = g_strdup (json_object_get_string_member (object, "source"));
    post_item->favourited = json_object_get_boolean_member (object, "favorited");
    if (json_object_has_member (object, "thumbnail_pic"))
    {
        post_item->thumbnail_pic = g_strdup (json_object_get_string_member (object,
                                                                            "thumbnail_pic"));
        post_item->bmiddle_pic = g_strdup (json_object_get_string_member (object,
                                                                          "bmiddle_pic"));
    }
    post_item->reposts_count = json_object_get_int_member (object, "reposts_count");
    post_item->comments_count = json_object_get_int_member (object, "comments_count");
    post_item->attitudes_count = json_object_get_int_member (object, "attitudes_count");

    user_object = json_object_get_object_member (object, "user");
    user->id = json_object_get_int_member (user_object, "id");
    user->idstr = g_strdup (json_object_get_string_member (user_object, "idstr"));
    user->name = g_strdup (json_object_get_string_member (user_object, "name"));
    user->location = g_strdup (json_object_get_string_member (user_object, "location"));
    user->profile_image_url = g_strdup (json_object_get_string_member (user_object,
                                                                      "profile_image_url"));

    post_item->user = user;
    row = GTK_WIDGET (gw_timeline_row_new (post_item));
    gtk_list_box_insert (GTK_LIST_BOX (priv->timeline_list), row, -1);
}

void
gw_timeline_list_get_home_timeline (GwTimelineList *self)
{
    const gchar *payload;
    GError *error = NULL;
    goffset payload_length;
    JsonNode *root_node;
    JsonParser *parser;
    RestProxy *proxy;
    RestProxyCall *call;

    proxy = oauth2_proxy_new_with_token ("1450991920",
                                         "2.005ugqwDURNMaB003aa34d9b828hsD",
                                         "https://api.weibo.com/oauth2/authorize",
                                         "https://api.weibo.com", FALSE);
    call = rest_proxy_new_call (proxy);
    rest_proxy_call_set_function (call, "2/statuses/home_timeline.json");
    rest_proxy_call_set_method (call, "GET");
    rest_proxy_call_add_params (call,
                                "access_token", "2.005ugqwDURNMaB003aa34d9b828hsD",
                                NULL);
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
gw_timeline_list_class_init (GwTimelineListClass *klass)
{
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/org/gnome/Weibo/gw-timeline-list.ui");
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  GwTimelineList, timeline_list);
}

static void
gw_timeline_list_init (GwTimelineList *self)
{
    GwTimelineListPrivate *priv;

    gtk_widget_init_template (GTK_WIDGET (self));

    priv = gw_timeline_list_get_instance_private (self);

    gtk_list_box_set_header_func (priv->timeline_list,
                                  (GtkListBoxUpdateHeaderFunc) listbox_update_header_func,
                                  NULL, NULL);
}

/**
 * gw_timeline_list_new:
 *
 * Create a new #GwTimelineList.
 *
 * Returns: (transfer full): a newly created #GwTimelineList
 */
GwTimelineList *
gw_timeline_list_new (void)
{
    return g_object_new (GW_TYPE_TIMELINE_LIST, NULL);
}
