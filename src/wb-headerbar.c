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
#include <json-glib/json-glib.h>
#include <rest/oauth2-proxy.h>

#include "wb-avatar-widget.h"
#include "wb-enums.h"
#include "wb-headerbar.h"
#include "wb-util.h"

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
    GtkWidget *avatar_image;
    GtkWidget *account_button;
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
users_show_finished_cb (RestProxyCall *call,
                        const GError *error,
                        GObject *weak_object,
                        gpointer user_data)
{
    const gchar *payload;
    goffset payload_length;
    GError *err = NULL;
    JsonNode *root_node;
    JsonParser *parser;
    WbHeaderbar *self;
    WbHeaderbarPrivate *priv;

    if (error != NULL)
    {
        g_error ("Error calling Weibo API(2/users/show): %s", error->message);
        return;
    }

    self = WB_HEADERBAR (user_data);
    priv = wb_headerbar_get_instance_private (self);

    payload = rest_proxy_call_get_payload (call);
    payload_length = rest_proxy_call_get_payload_length (call);

    parser = json_parser_new ();
    if (!json_parser_load_from_data (parser, payload, payload_length, &err))
    {
        g_warning ("json_parser_load_from_data () failed: %s (%s, %d)",
                   err->message,
                   g_quark_to_string (err->domain),
                   err->code);
        g_error_free (err);
    }

    root_node = json_parser_get_root (parser);
    if (JSON_NODE_HOLDS_OBJECT (root_node))
    {
        JsonObject *object;

        object = json_node_get_object (root_node);

        if (json_object_has_member (object, "profile_image_url"))
        {
            const gchar *uri;

            uri = json_object_get_string_member (object, "profile_image_url");
            wb_avatar_widget_setup (WB_AVATAR_WIDGET (priv->avatar_image),
                                    uri, TRUE);
            gtk_widget_show_all (priv->account_button);
        }
    }

    g_object_unref (parser);
}

static void
wb_headerbar_setup_account_button (WbHeaderbar *self)
{
    g_autofree gchar *access_token = NULL;
    g_autofree gchar *app_key = NULL;
    g_autofree gchar *uid = NULL;
    GError *error = NULL;
    RestProxy *proxy;
    RestProxyCall *call;

    access_token = wb_util_get_access_token ();
    app_key = wb_util_get_app_key ();
    uid = wb_util_get_uid ();

    proxy = oauth2_proxy_new_with_token (app_key, access_token,
                                         "https://api.weibo.com/oauth2/authorize",
                                         "https://api.weibo.com", FALSE);
    call = rest_proxy_new_call (proxy);
    rest_proxy_call_set_function (call, "2/users/show.json");
    rest_proxy_call_set_method (call, "GET");
    rest_proxy_call_add_param (call, "access_token", access_token);
    rest_proxy_call_add_param (call, "uid", uid);

    if (!rest_proxy_call_async (call, users_show_finished_cb, NULL, self, &error))
    {
        g_error ("API(2/users/show) call cancelled: %s",
                 error->message);
        g_error_free (error);
    }

    g_object_unref (call);
    g_object_unref (proxy);
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
    /* Ensure GTK+ private types used by the template definition
     * before calling gtk_widget_init_template()
     */
    g_type_ensure (WB_TYPE_AVATAR_WIDGET);

    gtk_widget_init_template (GTK_WIDGET (self));

    g_signal_connect (self, "notify::mode", G_CALLBACK (notify_mode_cb), NULL);

    wb_headerbar_setup_account_button (self);
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
                                                  avatar_image);
    gtk_widget_class_bind_template_child_private (widget_class, WbHeaderbar,
                                                  account_button);
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
