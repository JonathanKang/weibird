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

struct _WbComment
{
    GObject parent_instance;

    gboolean reply_comment;
    gchar *created_at;
    gchar *idstr;
    gchar *text;
    gint64 id;
    gint64 rootid;
    WbUser *user;
};

#define WB_TYPE_COMMENT (wb_comment_get_type ())

G_DECLARE_FINAL_TYPE (WbComment, wb_comment, WB, COMMENT, GObject)

WbComment *wb_comment_new (JsonObject *jobject);

G_END_DECLS
