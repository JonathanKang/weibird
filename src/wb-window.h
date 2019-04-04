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

#ifndef WB_WINDOW_H_
#define WB_WINDOW_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define WB_TYPE_WINDOW (wb_window_get_type ())
G_DECLARE_FINAL_TYPE (WbWindow, wb_window, WB, WINDOW, GtkApplicationWindow)

GtkWidget *wb_window_new (GtkApplication *application);

G_END_DECLS

#endif /* WB_WINDOW_H_ */
