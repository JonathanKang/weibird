/*
 *  Weibird - view and compose weibo
 *  copyright (c) 2019 jonathan kang <jonathankang@gnome.org>.
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

#include <glib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <rest/oauth2-proxy.h>
#include <webkit2/webkit2.h>

#include "wb-enums.h"
#include "wb-main-widget.h"
#include "wb-timeline-list.h"
#include "wb-tweet-detail-page.h"
#include "wb-tweet-row.h"
#include "wb-util.h"

enum
{
    PROP_0,
    PROP_MODE,
    N_PROPERTIES
};

struct _WbMainWidget
{
		GtkStack parent_instance;
};

typedef struct
{
    GtkWidget *login_box;
    GtkWidget *timeline;
    WbMainWidgetMode mode;
    WbTweetItem *tweet_item;
} WbMainWidgetPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbMainWidget, wb_main_widget, GTK_TYPE_STACK)

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };
static const gchar SETTINGS_SCHEMA[] = "com.jonathankang.Weibird";
static const gchar ACCESS_TOKEN[] = "access-token";
static const gchar EXPIRES_IN[] = "expires-in";
static const gchar UID[] = "uid";

GtkWidget *
wb_main_widget_get_timeline (WbMainWidget *self)
{
    WbMainWidgetPrivate *priv;

    g_return_val_if_fail (WB_MAIN_WIDGET (self), NULL);

    priv = wb_main_widget_get_instance_private (self);

    return priv->timeline;
}

void
wb_main_widget_set_mode (WbMainWidget *self,
                         WbMainWidgetMode mode)
{
    WbMainWidgetPrivate *priv;

    g_return_if_fail (WB_MAIN_WIDGET (self));

    priv = wb_main_widget_get_instance_private (self);

    if (priv->mode != mode)
    {
        priv->mode = mode;
        g_object_notify_by_pspec (G_OBJECT (self),
                                  obj_properties[PROP_MODE]);
    }
}

static gboolean
on_web_view_decide_policy (WebKitWebView *web_view,
                           WebKitPolicyDecision *decision,
                           WebKitPolicyDecisionType decision_type,
                           gpointer user_data)
{
    const gchar *requested_uri;
    const gchar *fragment;
    const gchar *query;
    g_autofree gchar *access_token = NULL;
    g_autofree gchar *code = NULL;
    GHashTable *key_value_pairs;
    SoupURI *uri;
    WebKitNavigationAction *action;
    WebKitURIRequest *request;

    if (decision_type != WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION)
    {
        goto default_behaviour;
    }

    action = webkit_navigation_policy_decision_get_navigation_action (WEBKIT_NAVIGATION_POLICY_DECISION (decision));
    request = webkit_navigation_action_get_request (action);
    requested_uri = webkit_uri_request_get_uri (request);
    if (!g_str_has_prefix (requested_uri, "https://api.weibo.com/oauth2/default.html"))
    {
        goto default_behaviour;
    }

    uri = soup_uri_new (requested_uri);
    fragment = soup_uri_get_fragment (uri);
    query = soup_uri_get_query (uri);

    if (fragment != NULL)
    {
        key_value_pairs = soup_form_decode (fragment);
        access_token = g_strdup (g_hash_table_lookup (key_value_pairs, "access_token"));

        g_hash_table_unref (key_value_pairs);
    }

    if (access_token != NULL)
    {
        goto ignore_request;
    }

    if (query != NULL)
    {
        key_value_pairs = soup_form_decode (query);

        code = g_strdup (g_hash_table_lookup (key_value_pairs, "code"));

        g_hash_table_unref (key_value_pairs);
    }

    if (code != NULL)
    {
        const gchar *payload;
        g_autofree gchar *app_key = NULL;
        g_autofree gchar *app_secret = NULL;
        gchar *uid = NULL;
        gint64 expires_in;
        GError *error = NULL;
        GError *tokens_error = NULL;
        GSettings *settings;
        gsize payload_length;
        guint status_code;
        JsonParser *parser;
        JsonObject *object;
        RestProxy *token_proxy;
        RestProxyCall *token_call;

        app_key = wb_util_get_app_key ();
        app_secret = wb_util_get_app_secret ();

        token_proxy = rest_proxy_new ("https://api.weibo.com/oauth2/access_token",
                                      FALSE);
        token_call = rest_proxy_new_call (token_proxy);

        rest_proxy_call_set_method (token_call, "POST");
        rest_proxy_call_add_header (token_call, "Content-Type",
                                    "application/x-www-form-urlencoded");
        rest_proxy_call_add_param (token_call, "client_id", app_key);
        rest_proxy_call_add_param (token_call, "client_secret", app_secret);
        rest_proxy_call_add_param (token_call, "grant_type", "authorization_code");
        rest_proxy_call_add_param (token_call, "redirect_uri",
                                   "https://api.weibo.com/oauth2/default.html");
        rest_proxy_call_add_param (token_call, "code", code);

        rest_proxy_call_sync (token_call, &error);
        if (error != NULL)
        {
            g_error ("Cannot make call: %s", error->message);
            g_error_free (error);

            goto ignore_request;
        }

        status_code = rest_proxy_call_get_status_code (token_call);
        if (status_code != 200)
        {
            g_error ("Expected status 200 when requesting access token, instead got status %d (%s)",
                     status_code,
                     rest_proxy_call_get_status_message (token_call));
            g_error_free (error);

            goto ignore_request;
        }

        payload = rest_proxy_call_get_payload (token_call);
        payload_length = rest_proxy_call_get_payload_length (token_call);

        parser = json_parser_new ();

        /* Parse the data we received */
        if (!json_parser_load_from_data (parser,
                                         payload, payload_length, &tokens_error))
        {
            g_warning ("json_parser_load_from_data () failed: %s (%s, %d)",
                       tokens_error->message,
                       g_quark_to_string (tokens_error->domain),
                       tokens_error->code);
            g_error_free (tokens_error);
            g_object_unref (parser);

            goto ignore_request;
        }

        object = json_node_get_object (json_parser_get_root (parser));
        if (!json_object_has_member (object, "access_token"))
        {
            g_warning ("Did not find access_token in JSON data");

            goto ignore_request;
        }

        /* Got the access token */
        access_token = g_strdup (json_object_get_string_member (object, "access_token"));
        expires_in = json_object_get_int_member (object, "expires_in");
        uid = g_strdup (json_object_get_string_member (object, "uid"));

        settings = g_settings_new (SETTINGS_SCHEMA);
        g_settings_set_string (settings, ACCESS_TOKEN, access_token);
        g_settings_set_int64 (settings, EXPIRES_IN, expires_in);
        g_settings_set_string (settings, UID, uid);

        g_free (uid);
        g_object_unref (parser);
        g_object_unref (settings);

        goto default_behaviour;
    }

ignore_request:
    webkit_policy_decision_ignore (decision);

    return TRUE;

default_behaviour:
    return FALSE;
}

static void
on_login_button_clicked (GtkWidget *button,
                         gpointer user_data)
{
    g_autofree gchar *app_key = NULL;
    gchar *uri;
    GtkWidget *content_area;
    GtkWidget *dialog;
    GtkWidget *toplevel;
    GtkWidget *web_view;
    RestProxy *proxy;
    WbTimelineList *timeline;
    WbMainWidget *main_widget;
    WbMainWidgetPrivate *priv;

    main_widget = WB_MAIN_WIDGET (user_data);
    priv = wb_main_widget_get_instance_private (main_widget);

    timeline = WB_TIMELINE_LIST (priv->timeline);

    web_view = webkit_web_view_new ();
    gtk_widget_set_hexpand (web_view, TRUE);
    gtk_widget_set_vexpand (web_view, TRUE);
    gtk_widget_set_size_request (web_view, 600, 400);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (main_widget));

    dialog = gtk_dialog_new ();
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (toplevel));
    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (gtk_widget_destroy), dialog);
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    gtk_container_add (GTK_CONTAINER (content_area), web_view);

    app_key = wb_util_get_app_key ();
    proxy = oauth2_proxy_new (app_key,
                              "https://api.weibo.com/oauth2/authorize",
                              "https://api.weibo.com", FALSE);

    uri = oauth2_proxy_build_login_url (OAUTH2_PROXY (proxy),
                                        "https://api.weibo.com/oauth2/default.html");

    /* Load the uri in web view */
    webkit_web_view_load_uri (WEBKIT_WEB_VIEW (web_view), uri);
    g_signal_connect (WEBKIT_WEB_VIEW (web_view), "decide-policy",
                      G_CALLBACK (on_web_view_decide_policy), proxy);

    gtk_widget_show_all (dialog);
    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_stack_set_visible_child (GTK_STACK (main_widget), priv->timeline);

    wb_timeline_list_get_home_timeline (timeline, FALSE);
}

static void
notify_mode_cb (GObject *object,
                GParamSpec *pspec,
                gpointer user_data)
{
    GtkStack *stack;
    GtkWidget *child;
    GtkWidget *toplevel;
    WbTweetDetailPage *detail;
    WbMainWidget *self = WB_MAIN_WIDGET (object);
    WbMainWidgetPrivate *priv = wb_main_widget_get_instance_private (self);

    stack = GTK_STACK (self);

    switch (priv->mode)
    {
        case WB_MAIN_WIDGET_MODE_LIST:
            child = gtk_stack_get_child_by_name (stack, "detail");

            if (child)
            {
                gtk_container_remove (GTK_CONTAINER (stack), child);
            }

            gtk_stack_set_visible_child (stack, priv->timeline);
            break;
        case WB_MAIN_WIDGET_MODE_DETAIL:
            {
                WbTweetItem *tweet_item;
                WbTweetItem *retweeted_item;
                WbTimelineList *timeline;

                timeline = WB_TIMELINE_LIST (priv->timeline);

                tweet_item = wb_timeline_list_get_tweet_item (timeline);
                retweeted_item = wb_timeline_list_get_retweeted_item (timeline);
                detail = wb_tweet_detail_page_new (tweet_item, retweeted_item);

                gtk_stack_add_named (stack, GTK_WIDGET (detail), "detail");
                gtk_stack_set_visible_child_name (stack, "detail");
            }
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
wb_main_widget_get_property (GObject *object,
                             guint prop_id,
                             GValue *value,
                             GParamSpec *pspec)
{
    WbMainWidget *self = WB_MAIN_WIDGET (object);
    WbMainWidgetPrivate *priv = wb_main_widget_get_instance_private (self);

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
wb_main_widget_set_property (GObject *object,
                             guint prop_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
    WbMainWidget *self = WB_MAIN_WIDGET (object);
    WbMainWidgetPrivate *priv = wb_main_widget_get_instance_private (self);

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
wb_main_widget_class_init (WbMainWidgetClass *klass)
{
    GObjectClass *gobject_class =  G_OBJECT_CLASS (klass);;
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->get_property = wb_main_widget_get_property;
    gobject_class->set_property = wb_main_widget_set_property;

    obj_properties[PROP_MODE] = g_param_spec_enum ("mode",
                                                   "Mode",
                                                   "Mode to determine which buttons to show",
                                                   WB_TYPE_MAIN_WIDGET_MODE,
                                                   WB_MAIN_WIDGET_MODE_LIST,
                                                   G_PARAM_READWRITE |
                                                   G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (gobject_class, N_PROPERTIES,
                                       obj_properties);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/com/jonathankang/Weibird/wb-main-widget.ui");
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbMainWidget, login_box);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbMainWidget, timeline);

    gtk_widget_class_bind_template_callback (widget_class, on_login_button_clicked);
}

static void
wb_main_widget_init (WbMainWidget *self)
{
    gchar *access_token;
    GSettings *settings;
    WbTimelineList *list;
    WbMainWidgetPrivate *priv;

    /* Ensure GTK+ private types used by the template definition
     * before calling gtk_widget_init_template()
     */
    g_type_ensure (WB_TYPE_TIMELINE_LIST);

    gtk_widget_init_template (GTK_WIDGET (self));

    priv = wb_main_widget_get_instance_private (self);
    list = WB_TIMELINE_LIST (priv->timeline);

    settings = g_settings_new (SETTINGS_SCHEMA);
    access_token = g_settings_get_string (settings, ACCESS_TOKEN);
    if (g_strcmp0 (access_token, "") != 0)
    {
        gtk_stack_set_visible_child (GTK_STACK (self), priv->timeline);
        wb_timeline_list_get_home_timeline (list, FALSE);
    }
    else
    {
        gtk_stack_set_visible_child (GTK_STACK (self), priv->login_box);
    }

    g_signal_connect (self, "notify::mode", G_CALLBACK (notify_mode_cb), NULL);

    g_free (access_token);
    g_object_unref (settings);
}

/**
 * wb_main_widget_new:
 *
 * Create a new #WbMainWidget.
 *
 * Returns: (transfer full): a newly created #WbMainWidget
 */
WbMainWidget *
wb_main_widget_new (void)
{
    return g_object_new (WB_TYPE_MAIN_WIDGET, NULL);
}
