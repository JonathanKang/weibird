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
#include "gw-window.h"

struct _GwApplication
{
    /*< private >*/
    GtkApplication parent_instance;
};

G_DEFINE_TYPE (GwApplication, gw_application, GTK_TYPE_APPLICATION)

static void
on_about (GSimpleAction *action,
          GVariant *variant,
          gpointer user_data)
{
    GtkApplication *application;
    GtkWindow *parent;

    const gchar *authors[] = {
        "Jonathan Kang <jonathankang@gnome.org>",
        NULL
    };

    application = GTK_APPLICATION (user_data);
    parent = gtk_application_get_active_window (application);

    gtk_show_about_dialog (parent,
                           "authors", authors,
                           "comments", "View and compose Weibo",
                           "copyright", "Copyright © 2018 Jonathan Kang",
                           "license-type", GTK_LICENSE_GPL_3_0,
                           "website", "https://github.com/JonathanKang/gnome-weibo",
                           NULL);
}

static void
on_quit (GSimpleAction *action,
         GVariant *variant,
         gpointer user_data)
{
    GApplication *application;

    application = G_APPLICATION (user_data);
    g_application_quit (application);
}

static GActionEntry actions[] = {
    { "about", on_about },
    { "quit", on_quit }
};

static void
gw_application_activate (GApplication *application)
{
    GtkWidget *window;

    window = gw_window_new (GTK_APPLICATION (application));
    gtk_widget_show (window);
}

static void
gw_application_startup (GApplication *application)
{
    g_action_map_add_action_entries (G_ACTION_MAP (application), actions,
                                     G_N_ELEMENTS (actions), application);

    /* Calls gtk_init() with no arguments. */
    G_APPLICATION_CLASS (gw_application_parent_class)->startup (application);
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
    app_class->startup = gw_application_startup;
}

GtkApplication *
gw_application_new (void)
{
    return g_object_new (GW_TYPE_APPLICATION,
                         "application-id", "org.gnome.Weibo", NULL);
}
