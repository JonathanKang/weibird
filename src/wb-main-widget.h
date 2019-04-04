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

#ifndef WB_MAIN_WIDGET_H_
#define WB_MAIN_WIDGET_H_

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * WbMainWidgetMode:
 * @WB_MAIN_WIDGET_MODE_LIST:
 * @WB_MAIN_WIDGET_MODE_DETAIL:
 *
 * The mode, mirroring the GlEventToolbar mode, used to show events.
 */
typedef enum
{
    WB_MAIN_WIDGET_MODE_LIST,
    WB_MAIN_WIDGET_MODE_DETAIL
} WbMainWidgetMode;

#define WB_TYPE_MAIN_WIDGET (wb_main_widget_get_type())

G_DECLARE_FINAL_TYPE (WbMainWidget, wb_main_widget, WB, MAIN_WIDGET, GtkStack)

GtkWidget *wb_main_widget_get_timeline (WbMainWidget *main_widget);
void wb_main_widget_set_mode (WbMainWidget *self, WbMainWidgetMode mode);
WbMainWidget *wb_main_widget_new (void);

G_END_DECLS


#endif /* WB_MAIN_WIDGET_H_ */
