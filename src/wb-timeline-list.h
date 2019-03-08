/*
 *  Weibird - view and compose weibo
 *  copyright (c) 2018-2019 jonathan kang <jonathankang@gnome.org>.
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

#ifndef WB_TIMELINE_LIST_H_
#define WB_TIMELINE_LIST_H_

#include <gtk/gtk.h>

#include "wb-tweet-item.h"

G_BEGIN_DECLS

#define WB_TYPE_TIMELINE_LIST (wb_timeline_list_get_type ())
G_DECLARE_FINAL_TYPE (WbTimelineList, wb_timeline_list, WB, TIMELINE_LIST, GtkBox)

WbTweetItem *wb_timeline_list_get_tweet_item (WbTimelineList *list);
GtkListBox *wb_timeline_list_get_listbox (WbTimelineList *list);
void wb_timeline_list_get_home_timeline (WbTimelineList *list, gboolean loading_more);
WbTimelineList *wb_timeline_list_new (void);

G_END_DECLS

#endif /* WB_TIMELINE_LIST_H_ */
