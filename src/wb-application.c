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

#include <gtk/gtk.h>

#include "config.h"
#include "wb-application.h"
#include "wb-window.h"

struct _WbApplication
{
    /*< private >*/
    GtkApplication parent_instance;
};

G_DEFINE_TYPE (WbApplication, wb_application, GTK_TYPE_APPLICATION)

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
                           "copyright", "Copyright © 2018-2019 Jonathan Kang",
                           "license-type", GTK_LICENSE_GPL_3_0,
                           "logo-icon-name", "com.jonathankang.Weibird",
                           "program-name", PROGRAM_NAME,
                           "version", PACKAGE_VERSION,
                           "website", PACKAGE_URL,
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
wb_application_activate (GApplication *application)
{
    GtkWidget *window;

    window = wb_window_new (GTK_APPLICATION (application));
    gtk_widget_show (window);
}

static void
wb_application_startup (GApplication *application)
{
    g_action_map_add_action_entries (G_ACTION_MAP (application), actions,
                                     G_N_ELEMENTS (actions), application);

    /* Calls gtk_init() with no arguments. */
    G_APPLICATION_CLASS (wb_application_parent_class)->startup (application);

    gtk_window_set_default_icon_name ("com.jonathankang.Weibird");
}

static void
wb_application_init (WbApplication *application)
{
}

static void
wb_application_class_init (WbApplicationClass *klass)
{
    GApplicationClass *app_class;

    app_class = G_APPLICATION_CLASS (klass);
    app_class->activate = wb_application_activate;
    app_class->startup = wb_application_startup;
}

GtkApplication *
wb_application_new (void)
{
    return g_object_new (WB_TYPE_APPLICATION,
                         "application-id", "com.jonathankang.Weibird", NULL);
}
