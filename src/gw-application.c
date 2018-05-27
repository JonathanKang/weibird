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

#include "gw-application.h"

struct _GwApplication
{
    /*< private >*/
    GtkApplication parent_instance;
};

G_DEFINE_TYPE (GwApplication, gw_application, GTK_TYPE_APPLICATION)

static void
gw_application_activate (GApplication *application)
{
    GtkWidget *window;

    window = gtk_application_window_new (GTK_APPLICATION (application));
    gtk_widget_show (window);
}

static void
gw_application_init (GwApplication *application)
{
}

static void
gw_application_class_init (GwApplicationClass *klass)
{
    GApplicationClass *app_class;

    app_class = G_APPLICATION_CLASS (klass);
    app_class->activate = gw_application_activate;
}

GtkApplication *
gw_application_new (void)
{
    return g_object_new (GW_TYPE_APPLICATION,
                         "application-id", "org.gnome.Weibo", NULL);
}
