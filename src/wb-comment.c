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

#include "wb-comment.h"

G_DEFINE_TYPE (WbComment, wb_comment, G_TYPE_OBJECT)

static void
wb_comment_parse_json_object (WbComment *self,
                              JsonObject *jobject)
{
    JsonObject *user_object;

    self->reply_comment = json_object_has_member (jobject, "reply_comment");
    self->created_at = g_strdup (json_object_get_string_member (jobject,
                                                                "created_at"));
    self->idstr = g_strdup (json_object_get_string_member (jobject, "idstr"));
    self->text = g_strdup (json_object_get_string_member (jobject, "text"));

    self->id = json_object_get_int_member (jobject, "id");
    if (json_object_has_member (jobject, "rootid"))
    {
        self->rootid = json_object_get_int_member (jobject, "rootid");
    }

    if (json_object_has_member (jobject, "user"))
    {
        user_object = json_object_get_object_member (jobject, "user");
        self->user = wb_user_new (user_object);
    }
}

static void
wb_comment_finalize (GObject *object)
{
    WbComment *self = WB_COMMENT (object);

    g_free (self->created_at);
    g_free (self->idstr);
    g_free (self->text);
    g_object_unref (self->user);

    G_OBJECT_CLASS (wb_comment_parent_class)->finalize (object);
}

static void
wb_comment_class_init (WbCommentClass *klass)
{
		GObjectClass *object_class = G_OBJECT_CLASS (klass);

		object_class->finalize = wb_comment_finalize;
}

static void
wb_comment_init (WbComment *self)
{
}

/**
 * wb_comment_new:
 *
 * Create a new #WbComment.
 *
 * Returns: (transfer full): a newly created #WbComment
 */
WbComment *
wb_comment_new (JsonObject *jobject)
{
    WbComment *comment;

    comment = g_object_new (WB_TYPE_COMMENT, NULL);

    wb_comment_parse_json_object (comment, jobject);

    return comment;
}
