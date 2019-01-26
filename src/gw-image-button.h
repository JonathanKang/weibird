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

#ifndef GW_IMAGE_BUTTON_H_
#define GW_IMAGE_BUTTON_H_

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GW_TYPE_IMAGE_BUTTON (gw_image_button_get_type ())
G_DECLARE_FINAL_TYPE (GwImageButton, gw_image_button, GW, IMAGE_BUTTON, GtkButton)

GtkWidget *gw_image_button_new (const gchar *uri);

G_END_DECLS

#endif /* GW_IMAGE_BUTTON_H_ */
