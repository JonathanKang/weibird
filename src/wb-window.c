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

#include "wb-enums.h"
#include "wb-headerbar.h"
#include "wb-main-widget.h"
#include "wb-timeline-list.h"
#include "wb-tweet-row.h"
#include "wb-tweet-detail-page.h"
#include "wb-util.h"
#include "wb-window.h"

struct _WbWindow
{
    /*< private >*/
    GtkApplicationWindow parent_instance;
};

typedef struct
{
    GtkWidget *main_widget;
    GtkWidget *headerbar;
} WbWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbWindow, wb_window, GTK_TYPE_APPLICATION_WINDOW)

GtkWidget *
wb_window_get_main_widget (WbWindow *self)
{
    WbWindowPrivate *priv;

    g_return_val_if_fail (WB_IS_WINDOW (self), NULL);
    g_print ("get main widget\n");

    priv = wb_window_get_instance_private (self);

    return priv->main_widget;
}

static void
on_action_radio (GSimpleAction *action,
                 GVariant *variant,
                 gpointer user_data)
{
    g_action_change_state (G_ACTION (action), variant);
}

static void
on_headerbar_mode (GSimpleAction *action,
                   GVariant *variant,
                   gpointer user_data)
{
    const gchar *mode;
    GEnumClass *eclass;
    GEnumValue *evalue;
    WbWindowPrivate *priv;

    priv = wb_window_get_instance_private (WB_WINDOW (user_data));

    mode = g_variant_get_string (variant, NULL);
    eclass = g_type_class_ref (WB_TYPE_HEADERBAR_MODE);
    evalue = g_enum_get_value_by_nick (eclass, mode);

    if (evalue->value == WB_HEADERBAR_MODE_LIST)
    {
        /* Switch back to list mode when the back button is clicked. */
        WbMainWidget *main_widget;

        main_widget = WB_MAIN_WIDGET (priv->main_widget);

        wb_main_widget_set_mode (main_widget, WB_MAIN_WIDGET_MODE_LIST);
    }

    g_simple_action_set_state (action, variant);

    g_type_class_unref (eclass);
}

static void
on_view_mode (GSimpleAction *action,
              GVariant *variant,
              gpointer user_data)
{
    const gchar *mode;
    GEnumClass *eclass;
    GEnumValue *evalue;
    WbHeaderbar *headerbar;
    WbMainWidget *main_widget;
    WbWindowPrivate *priv;

    priv = wb_window_get_instance_private (WB_WINDOW (user_data));
    headerbar = WB_HEADERBAR (priv->headerbar);
    main_widget = WB_MAIN_WIDGET (priv->main_widget);

    mode = g_variant_get_string (variant, NULL);
    eclass = g_type_class_ref (WB_TYPE_HEADERBAR_MODE);
    evalue = g_enum_get_value_by_nick (eclass, mode);

    switch (evalue->value)
    {
        case WB_MAIN_WIDGET_MODE_LIST:
            wb_headerbar_set_mode (headerbar, WB_HEADERBAR_MODE_LIST);
            break;
        case WB_MAIN_WIDGET_MODE_DETAIL:
            wb_headerbar_set_mode (headerbar, WB_HEADERBAR_MODE_DETAIL);
            wb_main_widget_set_mode (main_widget, WB_MAIN_WIDGET_MODE_DETAIL);
            break;
        case WB_MAIN_WIDGET_MODE_PROFILE:
            wb_headerbar_set_mode (headerbar, WB_HEADERBAR_MODE_PROFILE);
            wb_main_widget_set_mode (main_widget, WB_MAIN_WIDGET_MODE_PROFILE);
            break;
        default:
            g_assert_not_reached ();
            break;
    }

    g_simple_action_set_state (action, variant);

    g_type_class_unref (eclass);
}

static GActionEntry actions[] = {
    { "headerbar-mode", on_action_radio, "s", "'list'", on_headerbar_mode },
    { "view-mode", on_view_mode, "s", "'list'", on_view_mode }
};

static void
wb_window_init (WbWindow *window)
{
    GdkScreen *screen;
    GtkCssProvider *provider;

    /* Ensure GTK+ private types used by the template definition
     * before calling gtk_widget_init_template()
     */
    g_type_ensure (WB_TYPE_HEADERBAR);
    g_type_ensure (WB_TYPE_MAIN_WIDGET);
    g_type_ensure (WB_TYPE_TIMELINE_LIST);

    gtk_widget_init_template (GTK_WIDGET (window));

    g_action_map_add_action_entries (G_ACTION_MAP (window), actions,
                                     G_N_ELEMENTS (actions), window);

    screen = gdk_screen_get_default ();
    provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_resource (provider, "/com/jonathankang/Weibird/wb-style.css");
    gtk_style_context_add_provider_for_screen (screen,
                                               GTK_STYLE_PROVIDER (provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref (provider);
}
static void
wb_window_class_init (WbWindowClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/com/jonathankang/Weibird/wb-window.ui");
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbWindow, main_widget);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbWindow, headerbar);
}

GtkWidget *
wb_window_new (GtkApplication *application)
{
    return g_object_new (WB_TYPE_WINDOW,
                         "application", application, NULL);
}
