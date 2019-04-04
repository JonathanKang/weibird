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

#include <gtk/gtk.h>

#include "wb-comment.h"

G_BEGIN_DECLS

#define WB_TYPE_COMMENT_ROW (wb_comment_row_get_type ())

G_DECLARE_FINAL_TYPE (WbCommentRow, wb_comment_row, WB, COMMENT_ROW, GtkListBoxRow)

WbComment *wb_comment_row_get_comment (WbCommentRow *row);
void wb_comment_row_insert_reply (WbCommentRow *row, WbComment *comment);
WbCommentRow *wb_comment_row_new (WbComment *comment);

G_END_DECLS
