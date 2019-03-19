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

#ifndef WB_TWEET_ROW_H_
#define WB_TWEET_ROW_H_

#include <gtk/gtk.h>

#include "wb-tweet-item.h"

G_BEGIN_DECLS

#define WB_TYPE_TWEET_ROW (wb_tweet_row_get_type ())
G_DECLARE_FINAL_TYPE (WbTweetRow, wb_tweet_row, WB, TWEET_ROW, GtkListBoxRow)

GtkWidget *wb_tweet_row_get_comment_button (WbTweetRow *row);
WbTweetItem *wb_tweet_row_get_tweet_item (WbTweetRow *row);
WbTweetItem *wb_tweet_row_get_retweeted_item (WbTweetRow *row);
void wb_tweet_row_insert_retweeted_item (WbTweetRow *row,
                                         GtkWidget *retweeted_widget);
WbTweetRow *wb_tweet_row_new (WbTweetItem *tweet_item,
                              WbTweetItem *retweet_item,
                              gboolean retweet);

G_END_DECLS

#endif /* WB_TWEET_ROW_H_ */
