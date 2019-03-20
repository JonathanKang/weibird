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

#pragma once

#include <gtk/gtk.h>

#include "wb-comment-row.h"
#include "wb-tweet-item.h"

G_BEGIN_DECLS

#define WB_TYPE_COMMENT_LIST (wb_comment_list_get_type ())

G_DECLARE_FINAL_TYPE (WbCommentList, wb_comment_list, WB, COMMENT_LIST, GtkListBox)

void wb_comment_list_set_tweet_id (WbCommentList *list, const gchar *tweet_id);
void wb_comment_list_insert_comment_widget (WbCommentList *list,
                                            WbCommentRow *comment_widget);
WbCommentList *wb_comment_list_new (void);

G_END_DECLS
