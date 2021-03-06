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

#include <json-glib/json-glib.h>

#include "wb-tweet-item.h"
#include "wb-user.h"
#include "wb-util.h"

G_DEFINE_TYPE (WbTweetItem, wb_tweet_item, G_TYPE_OBJECT)

static void
parse_pic_uri (JsonArray *array,
               guint index,
               JsonNode *element_node,
               gpointer user_data)
{
    const gchar *thumbnail;
    gchar *uri;
    JsonObject *object;
    WbTweetItem *self = WB_TWEET_ITEM (user_data);

    object = json_node_get_object (element_node);

    thumbnail = json_object_get_string_member (object, "thumbnail_pic");
    uri = g_strdup (thumbnail);
    g_array_append_val (self->picuri_array, uri);
}

static void
wb_tweet_item_parse_json_object (WbTweetItem *self,
                                 JsonObject *object)
{
    JsonArray *pic_array;
    JsonObject *user_object;

    self->created_at = g_strdup (json_object_get_string_member (object, "created_at"));
    self->id = json_object_get_int_member (object, "id");
    self->mid = json_object_get_int_member (object, "mid");
    self->idstr = g_strdup (json_object_get_string_member (object, "idstr"));
    self->text = g_strdup (json_object_get_string_member (object, "text"));
    self->source = g_strdup (json_object_get_string_member (object, "source"));
    self->favourited = json_object_get_boolean_member (object, "favorited");
    self->reposts_count = json_object_get_int_member (object, "reposts_count");
    self->comments_count = json_object_get_int_member (object, "comments_count");
    self->attitudes_count = json_object_get_int_member (object, "attitudes_count");

    /* Parse the uri for each picture if there is any */
    self->picuri_array = g_array_new (FALSE, FALSE, sizeof (gchar *));
    pic_array = json_object_get_array_member (object, "pic_urls");
    if (json_array_get_length (pic_array) != 0)
    {
        json_array_foreach_element (pic_array, parse_pic_uri, self);
    }

    user_object = json_object_get_object_member (object, "user");
    self->user = wb_user_new (user_object);
}

static void
wb_tweet_item_finalize (GObject *object)
{
    gint i;
    WbTweetItem *self = (WbTweetItem *)object;

    for (i = 0; i < self->picuri_array->len; i++)
    {
        g_free (g_array_index (self->picuri_array, gchar *, i));
    }
    g_array_free (self->picuri_array, TRUE);
    g_free (self->created_at);
    g_free (self->idstr);
    g_free (self->text);
    g_free (self->source);
    g_object_unref (self->user);

    G_OBJECT_CLASS (wb_tweet_item_parent_class)->finalize (object);
}

static void
wb_tweet_item_class_init (WbTweetItemClass *klass)
{
		GObjectClass *object_class = G_OBJECT_CLASS (klass);

		object_class->finalize = wb_tweet_item_finalize;
}

static void
wb_tweet_item_init (WbTweetItem *self)
{
}

/**
 * wb_tweet_item_new:
 *
 * Create a new #WbTweetItem.
 *
 * Returns: (transfer full): a newly created #WbTweetItem
 */
WbTweetItem *
wb_tweet_item_new (JsonObject *jobject)
{
    WbTweetItem *item;

    item = g_object_new (WB_TYPE_TWEET_ITEM, NULL);

    wb_tweet_item_parse_json_object (item, jobject);

    return item;
}
