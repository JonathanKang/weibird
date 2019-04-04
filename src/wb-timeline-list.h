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

#ifndef WB_TIMELINE_LIST_H_
#define WB_TIMELINE_LIST_H_

#include <gtk/gtk.h>

#include "wb-tweet-item.h"

G_BEGIN_DECLS

#define WB_TYPE_TIMELINE_LIST (wb_timeline_list_get_type ())
G_DECLARE_FINAL_TYPE (WbTimelineList, wb_timeline_list, WB, TIMELINE_LIST, GtkBox)

WbTweetItem *wb_timeline_list_get_tweet_item (WbTimelineList *list);
WbTweetItem *wb_timeline_list_get_retweeted_item (WbTimelineList *list);
GtkListBox *wb_timeline_list_get_listbox (WbTimelineList *list);
void wb_timeline_list_get_home_timeline (WbTimelineList *list, gboolean loading_more);
WbTimelineList *wb_timeline_list_new (void);

G_END_DECLS

#endif /* WB_TIMELINE_LIST_H_ */
