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

G_BEGIN_DECLS

struct _WbUser
{
		GObject parent_instance;

    gint64 id;
    gchar *idstr;
    gchar *name;
    gchar *nickname;
    gchar *location;
    gchar *description;
    gchar *url;
    gchar *profile_image_url;
    gchar *gender;
    gint followers_count;
    gint friends_count;
    gint statuses_count;
    gint favourites_count;
    gchar *created_at;
    gboolean verified;
    gboolean follow_me;
};

#define WB_TYPE_USER (wb_user_get_type ())

G_DECLARE_FINAL_TYPE (WbUser, wb_user, WB, USER, GObject)

WbUser *wb_user_new (JsonObject *jobject);

G_END_DECLS
