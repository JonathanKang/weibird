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

#ifndef WB_TWEET_DETAIL_PAGE_H_
#define WB_TWEET_DETAIL_PAGE_H_

#pragma once

#include <gtk/gtk.h>

#include "wb-tweet-item.h"

G_BEGIN_DECLS

#define WB_TYPE_TWEET_DETAIL_PAGE (wb_tweet_detail_page_get_type())

G_DECLARE_FINAL_TYPE (WbTweetDetailPage, wb_tweet_detail_page, WB, TWEET_DETAIL_PAGE, GtkBox)

WbTweetDetailPage *wb_tweet_detail_page_new (WbTweetItem *tweet_item,
                                             WbTweetItem *retweeted_item);

G_END_DECLS

#endif /* WB_TWEET_DETAIL_PAGE_H_ */
