/*
 *  Weibird - view and compose weibo
 *  copyright (c) 2019 jonathan kang <jonathankang@gnome.org>.
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

#pragma once

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "wb-user.h"

G_BEGIN_DECLS

struct _WbComment
{
    GObject parent_instance;

    gchar *created_at;
    gint64 id;
    gchar *text;
    gchar *idstr;
    WbUser *user;
};

#define WB_TYPE_COMMENT (wb_comment_get_type ())

G_DECLARE_FINAL_TYPE (WbComment, wb_comment, WB, COMMENT, GObject)

WbComment *wb_comment_new (JsonObject *jobject);

G_END_DECLS
