/*
 *  Weibird - View and compose weibo
 *  Copyright (C) 2019 Jonathan Kang <jonathankang@gnome.org>.
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
#include "wb-profile-page.h"
#include "wb-util.h"

struct _WbProfilePage
{
		GtkBox parent_instance;
};

enum
{
    PROP_0,
    PROP_UID,
    N_PROPERTIES
};

typedef struct
{
    gchar *avatar_uri;
    gchar *description;
    gchar *nickname;
    gchar *screen_name;
    gchar *uid;
    gint followings_count;
    gint followers_count;
    gint tweets_count;
    GtkWidget *avatar;
    GtkWidget *nickname_label;
    GtkWidget *screenname_label;
    GtkWidget *intro_label;
    GtkWidget *followings_label;
    GtkWidget *followers_label;
    GtkWidget *tweets_label;
} WbProfilePagePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbProfilePage, wb_profile_page, GTK_TYPE_BOX)

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
wb_profile_page_parse_json_object (WbProfilePage *self,
                                   JsonObject *object)
{
    WbProfilePagePrivate *priv;

    priv = wb_profile_page_get_instance_private (self);

    priv->avatar_uri = g_strdup (json_object_get_string_member (object, "profile_image_url"));
    priv->screen_name = g_strdup (json_object_get_string_member (object, "screen_name"));
    priv->nickname = g_strdup (json_object_get_string_member (object, "name"));
    priv->description = g_strdup (json_object_get_string_member (object, "description"));
    priv->followings_count = json_object_get_int_member (object, "friends_count");
    priv->followers_count = json_object_get_int_member (object, "followers_count");
    priv->tweets_count = json_object_get_int_member (object, "statuses_count");
}

static void
wb_profile_page_call_users_show (WbProfilePage *self)
{
    const gchar *payload;
    g_autofree gchar *access_token = NULL;
    g_autofree gchar *app_key = NULL;
    GError *error = NULL;
    goffset payload_length;
    JsonNode *root_node;
    JsonParser *parser;
    RestProxy *proxy;
    RestProxyCall *call;
    WbProfilePagePrivate *priv;

    priv = wb_profile_page_get_instance_private (self);

    access_token = wb_util_get_access_token ();
    app_key = wb_util_get_app_key ();

    proxy = oauth2_proxy_new_with_token (app_key, access_token,
                                         "https://api.weibo.com/oauth2/authorize",
                                         "https://api.weibo.com", FALSE);
    call = rest_proxy_new_call (proxy);
    rest_proxy_call_set_function (call, "2/users/show.json");
    rest_proxy_call_set_method (call, "GET");
    rest_proxy_call_add_param (call, "access_token", access_token);
    rest_proxy_call_add_param (call, "uid", priv->uid);

    rest_proxy_call_sync (call, &error);
    if (error != NULL)
    {
        g_error ("Error calling Weibo API(users/show): %s", error->message);
        g_error_free (error);
    }

    payload = rest_proxy_call_get_payload (call);
    payload_length = rest_proxy_call_get_payload_length (call);

    parser = json_parser_new ();
    if (!json_parser_load_from_data (parser, payload, payload_length, &error))
    {
        g_warning ("json_parser_load_from_data () failed: %s (%s, %d)",
                   error->message,
                   g_quark_to_string (error->domain),
                   error->code);
        g_error_free (error);
    }

    root_node = json_parser_get_root (parser);
    if (JSON_NODE_HOLDS_OBJECT (root_node))
    {
        JsonObject *object;

        object = json_node_get_object (root_node);

        wb_profile_page_parse_json_object (self, object);
    }

    g_object_unref (call);
    g_object_unref (parser);
    g_object_unref (proxy);
}

void
wb_profile_page_setup (WbProfilePage *self)
{
    gchar *markup;
    WbProfilePagePrivate *priv = wb_profile_page_get_instance_private (self);

    /* Call users/show API here. */
    wb_profile_page_call_users_show (self);

    wb_avatar_widget_setup (WB_AVATAR_WIDGET (priv->avatar), priv->avatar_uri);
    gtk_label_set_text (GTK_LABEL (priv->screenname_label), priv->screen_name);
    gtk_label_set_text (GTK_LABEL (priv->nickname_label), priv->nickname);
    gtk_label_set_text (GTK_LABEL (priv->intro_label), priv->description);

    markup = g_markup_printf_escaped ("<b>%d</b> Following", priv->followings_count);
    gtk_label_set_markup (GTK_LABEL (priv->followings_label), markup);
    g_free (markup);

    markup = g_markup_printf_escaped ("<b>%d</b> Followers", priv->followers_count);
    gtk_label_set_markup (GTK_LABEL (priv->followers_label), markup);
    g_free (markup);

    markup = g_markup_printf_escaped ("<b>%d</b> Tweets", priv->tweets_count);
    gtk_label_set_markup (GTK_LABEL (priv->tweets_label), markup);
    g_free (markup);
}

static void
wb_profile_page_finalize (GObject *object)
{
    WbProfilePage *self = (WbProfilePage *)object;
    WbProfilePagePrivate *priv = wb_profile_page_get_instance_private (self);

    g_free (priv->uid);

    G_OBJECT_CLASS (wb_profile_page_parent_class)->finalize (object);
}

static void
wb_profile_page_get_property (GObject *object,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
    WbProfilePage *self = WB_PROFILE_PAGE (object);
    WbProfilePagePrivate *priv = wb_profile_page_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_UID:
            g_value_set_string (value, priv->uid);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
wb_profile_page_set_property (GObject *object,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
    WbProfilePage *self = WB_PROFILE_PAGE (object);
    WbProfilePagePrivate *priv = wb_profile_page_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_UID:
            priv->uid = g_value_dup_string (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
wb_profile_page_class_init (WbProfilePageClass *klass)
{
		GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

		object_class->finalize = wb_profile_page_finalize;
		object_class->get_property = wb_profile_page_get_property;
		object_class->set_property = wb_profile_page_set_property;

    obj_properties[PROP_UID] = g_param_spec_string ("uid",
                                                    "uid",
                                                    "User ID",
                                                    "0",
                                                    G_PARAM_READWRITE |
                                                    G_PARAM_CONSTRUCT_ONLY |
                                                    G_PARAM_STATIC_STRINGS);
    g_object_class_install_properties (object_class, N_PROPERTIES,
                                       obj_properties);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/com/jonathankang/Weibird/wb-profile-page.ui");
    gtk_widget_class_bind_template_child_private (widget_class, WbProfilePage,
                                                  avatar);
    gtk_widget_class_bind_template_child_private (widget_class, WbProfilePage,
                                                  nickname_label);
    gtk_widget_class_bind_template_child_private (widget_class, WbProfilePage,
                                                  screenname_label);
    gtk_widget_class_bind_template_child_private (widget_class, WbProfilePage,
                                                  intro_label);
    gtk_widget_class_bind_template_child_private (widget_class, WbProfilePage,
                                                  followings_label);
    gtk_widget_class_bind_template_child_private (widget_class, WbProfilePage,
                                                  followers_label);
    gtk_widget_class_bind_template_child_private (widget_class, WbProfilePage,
                                                  tweets_label);
}

static void
wb_profile_page_init (WbProfilePage *self)
{
    WbProfilePagePrivate *priv = wb_profile_page_get_instance_private (self);

    gtk_widget_init_template (GTK_WIDGET (self));

    priv->uid = NULL;
}

/**
 * wb_profile_page_new:
 *
 * Create a new #WbProfilePage.
 *
 * Returns: (transfer full): a newly created #WbProfilePage
 */
WbProfilePage *
wb_profile_page_new (const gchar *uid)
{
    return g_object_new (WB_TYPE_PROFILE_PAGE,
                         "uid", uid,
                         NULL);
}
