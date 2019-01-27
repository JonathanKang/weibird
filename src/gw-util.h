/*
 *  GNOME Weibo - view and compose weibo
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

#ifndef GW_UTIL_H_
#define GW_UTIL_H_

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "gw-timeline-list.h"

G_BEGIN_DECLS

gchar *gw_util_format_time_string (gchar *time);
gchar *gw_util_format_source_string (gchar *source);
gchar *gw_util_thumbnail_to_original (const gchar *thumbnail);
void gw_util_parse_weibo_post (JsonObject *object, GwPostItem *post_item);

G_BEGIN_DECLS

#endif /* GW_UTIL_H_ */
