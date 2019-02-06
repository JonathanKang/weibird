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

#ifndef WB_MEDIA_IMAGE_H_
#define WB_MEDIA_IMAGE_H_

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef enum
{
    WB_MEDIA_TYPE_AVATAR,
    WB_MEDIA_TYPE_IMAGE
} WbMediaType;

#define WB_TYPE_MEDIA_IMAGE (wb_media_image_get_type())

G_DECLARE_FINAL_TYPE (WbMediaImage, wb_media_image, WB, MEDIA_IMAGE, GtkImage)

WbMediaType wb_media_image_get_media_type (WbMediaImage *self);
GdkPixbuf *wb_media_image_get_pixbuf (WbMediaImage *self);
WbMediaImage *wb_media_image_new (WbMediaType type,
                                  const gchar *uri,
                                  gint width,
                                  gint height);

G_END_DECLS

#endif /* WB_MEDIA_IMAGE_H_ */
