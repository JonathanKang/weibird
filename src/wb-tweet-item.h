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

#pragma once

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "wb-user.h"

G_BEGIN_DECLS

struct _WbTweetItem
{
		GObject parent_instance;

    gchar *created_at;
    gint64 id;
    gint64 mid;
    gchar *idstr;

    gchar *text;
    gchar *source;

    gboolean favourited;
    gchar *thumbnail_pic;
    gchar *bmiddle_pic;
    GArray *picuri_array;

    gint reposts_count;
    gint comments_count;
    gint attitudes_count;

    WbUser *user;
};

#define WB_TYPE_TWEET_ITEM (wb_tweet_item_get_type ())

G_DECLARE_FINAL_TYPE (WbTweetItem, wb_tweet_item, WB, TWEET_ITEM, GObject)

gint64 wb_tweet_item_get_id (WbTweetItem *tweet_item);
const gchar *wb_tweet_item_get_idstr (WbTweetItem *tweet_item);
WbTweetItem *wb_tweet_item_new (JsonObject *jobject);

G_END_DECLS
