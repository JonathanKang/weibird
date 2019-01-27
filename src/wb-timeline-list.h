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

G_BEGIN_DECLS

typedef struct
{
    gint64 id;
    gchar *idstr;
    gchar *name;
    gchar *nickname;
    gchar *location;
    gchar *description;
    gchar *url;
    gchar *profile_image_url;
    gchar *gender;
    gint followers_count;
    gint friends_count;
    gint statuses_count;
    gint favourites_count;
    gchar *created_at;
    gboolean verified;
    gboolean follow_me;
} WbUser;

typedef struct
{
    gchar *created_at;
    gint64 id;
    gint64 mid;
    gchar *idstr;

    gchar *text;
    gchar *source;

    gboolean favourited;
    gchar *thumbnail_pic;
    gchar *bmiddle_pic;
    GArray *picuri_array;

    gint reposts_count;
    gint comments_count;
    gint attitudes_count;

    WbUser *user;
} WbPostItem;

#define WB_TYPE_TIMELINE_LIST (wb_timeline_list_get_type ())
G_DECLARE_FINAL_TYPE (WbTimelineList, wb_timeline_list, WB, TIMELINE_LIST, GtkBox)

void wb_timeline_list_get_home_timeline (WbTimelineList *list, gboolean loading_more);
WbTimelineList *wb_timeline_list_new (void);

G_END_DECLS

#endif /* WB_TIMELINE_LIST_H_ */
