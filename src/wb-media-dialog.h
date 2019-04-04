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

#ifndef WB_MEDIA_DIALOG_H_
#define WB_MEDIA_DIALOG_H_

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define WB_TYPE_MEDIA_DIALOG (wb_media_dialog_get_type ())
G_DECLARE_FINAL_TYPE (WbMediaDialog, wb_media_dialog, WB, MEDIA_DIALOG, GtkWindow)

GtkWidget *wb_media_dialog_get_frame (WbMediaDialog *dialog);
WbMediaDialog *wb_media_dialog_new (GArray *images, gint nth_media, GtkWidget *cur_image);

G_END_DECLS

#endif /* WB_MEDIA_DIALOG_H_ */
