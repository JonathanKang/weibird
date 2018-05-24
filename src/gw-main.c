/*
 *  GNOME Weibo - View and compose weibo
 *  Copyright (C) 2018 Jonathan Kang <jonathankang@gnome.org>.
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

#include <gtk/gtk.h>

static void
on_activate (GApplication *app,
             gpointer user_data)
{
    GtkWidget *window;

    window = gtk_application_window_new (GTK_APPLICATION (app));
    gtk_widget_show (window);
}

int
main (int argc, char **argv)
{
    GtkApplication *application;
    int status;

    application = gtk_application_new ("org.gnome.Weibo",
                                       G_APPLICATION_FLAGS_NONE);
    g_signal_connect (application, "activate",
                      G_CALLBACK (on_activate), NULL);

    status = g_application_run (G_APPLICATION (application), argc, argv);

    g_object_unref (application);

    return status;
}
