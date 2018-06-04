/*
 *  gnome weibo - view and compose weibo
 *  copyright (c) 2018 jonathan kang <jonathankang@gnome.org>.
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

#include <gtk/gtk.h>

#include "gw-window.h"

struct _GwWindow
{
    /*< private >*/
    GtkApplicationWindow parent_instance;
};

G_DEFINE_TYPE (GwWindow, gw_window, GTK_TYPE_APPLICATION_WINDOW)

static void
gw_window_init (GwWindow *window)
{
}

static void
gw_window_class_init (GwWindowClass *klass)
{
}

GtkWidget *
gw_window_new (GtkApplication *application)
{
    return g_object_new (GW_TYPE_WINDOW,
                         "application", application, NULL);
}
