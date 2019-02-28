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
