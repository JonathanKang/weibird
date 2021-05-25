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

#ifndef WB_UTIL_H_
#define WB_UTIL_H_

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>

#include "wb-timeline-list.h"

G_BEGIN_DECLS

#define MAX_WIDTH 1000
#define MAX_HEIGHT 800

extern SoupSession *SOUPSESSION;

void wb_util_init_soup_session (void);
void wb_util_finalize_soup_session (void);
gchar *wb_util_format_time_string (const gchar *time);
gchar *wb_util_format_source_string (const gchar *source);
gchar *wb_util_thumbnail_to_middle (const gchar *thumbnail);
gchar *wb_util_thumbnail_to_original (const gchar *thumbnail);
GtkWidget *wb_util_scale_image (GdkPixbuf *pixbuf, gint *width, gint *height);
gchar *wb_util_get_access_token (void);
gchar *wb_util_get_app_key (void);
gchar *wb_util_get_app_secret (void);

G_BEGIN_DECLS

#endif /* WB_UTIL_H_ */
