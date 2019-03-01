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

#include "wb-enums.h"
#include "wb-headerbar.h"

enum
{
    PROP_0,
    PROP_MODE,
    N_PROPERTIES
};

struct _WbHeaderbar
{
    /*< private >*/
    GtkHeaderBar parent_instance;
};

typedef struct
{
    GtkWidget *back_button;
    GtkWidget *pri_menu;
    WbHeaderbarMode mode;
} WbHeaderbarPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbHeaderbar, wb_headerbar, GTK_TYPE_HEADER_BAR)

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

void
wb_headerbar_set_mode (WbHeaderbar *self,
                       WbHeaderbarMode mode)
{
    WbHeaderbarPrivate *priv = wb_headerbar_get_instance_private (self);

    g_return_if_fail (WB_HEADERBAR (self));

    if (priv->mode != mode)
    {
        priv->mode = mode;
        g_object_notify_by_pspec (G_OBJECT (self), obj_properties[PROP_MODE]);
    }
}

static void
back_button_clicked_cb (GtkButton *button,
                        gpointer user_data)
{
    wb_headerbar_set_mode (WB_HEADERBAR (user_data), WB_HEADERBAR_MODE_LIST);
}

static void
notify_mode_cb (GObject *object,
                GParamSpec *pspec,
                gpointer user_data)
{
    GtkWidget *toplevel;
    WbHeaderbar *self = WB_HEADERBAR (object);
    WbHeaderbarPrivate *priv = wb_headerbar_get_instance_private (self);

    switch (priv->mode)
    {
        case WB_HEADERBAR_MODE_LIST:
            gtk_widget_hide (priv->back_button);
            break;
        case WB_HEADERBAR_MODE_DETAIL:
            gtk_widget_show (priv->back_button);
            break;
        default:
            g_assert_not_reached ();
            break;
    }

    /* Propagate change to WbWindow. */
    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

    if (gtk_widget_is_toplevel (toplevel))
    {
        GAction *mode;
        GEnumClass *eclass;
        GEnumValue *evalue;

        mode = g_action_map_lookup_action (G_ACTION_MAP (toplevel),
                                           "headerbar-mode");
        eclass = g_type_class_ref (WB_TYPE_HEADERBAR_MODE);
        evalue = g_enum_get_value (eclass, priv->mode);

        g_action_activate (mode, g_variant_new_string (evalue->value_nick));

        g_type_class_unref (eclass);
    }
    else
    {
        g_error ("Widget not in toplevel window, not switching headerbar mode");
    }
}

static void
wb_headerbar_get_property (GObject *object,
                           guint prop_id,
                           GValue *value,
                           GParamSpec *pspec)
{
    WbHeaderbar *self = WB_HEADERBAR (object);
    WbHeaderbarPrivate *priv = wb_headerbar_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_MODE:
            g_value_set_enum (value, priv->mode);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
wb_headerbar_set_property (GObject *object,
                           guint prop_id,
                           const GValue *value,
                           GParamSpec *pspec)
{
    WbHeaderbar *self = WB_HEADERBAR (object);
    WbHeaderbarPrivate *priv = wb_headerbar_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_MODE:
            priv->mode = g_value_get_enum (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
wb_headerbar_init (WbHeaderbar *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));

    g_signal_connect (self, "notify::mode", G_CALLBACK (notify_mode_cb), NULL);
}

static void
wb_headerbar_class_init (WbHeaderbarClass *klass)
{
    GObjectClass *gobject_class =  G_OBJECT_CLASS (klass);;
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);;

    gobject_class->get_property = wb_headerbar_get_property;
    gobject_class->set_property = wb_headerbar_set_property;

    obj_properties[PROP_MODE] = g_param_spec_enum ("mode",
                                                   "Mode",
                                                   "Mode to determine which buttons to show",
                                                   WB_TYPE_HEADERBAR_MODE,
                                                   WB_HEADERBAR_MODE_LIST,
                                                   G_PARAM_READWRITE |
                                                   G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (gobject_class, N_PROPERTIES,
                                       obj_properties);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/com/jonathankang/Weibird/wb-headerbar.ui");
    gtk_widget_class_bind_template_child_private (widget_class, WbHeaderbar,
                                                  back_button);
    gtk_widget_class_bind_template_child_private (widget_class, WbHeaderbar,
                                                  pri_menu);
    gtk_widget_class_bind_template_callback (widget_class, back_button_clicked_cb);
}

WbHeaderbar *
wb_headerbar_new (void)
{
    return g_object_new (WB_TYPE_HEADERBAR, NULL);
}
