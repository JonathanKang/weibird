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
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <rest/oauth2-proxy.h>
#include <webkit2/webkit2.h>

#include "gw-timeline-list.h"

struct _GwTimelineList
{
    /*< private >*/
    GtkBox parent_instance;
};

typedef struct
{
    GtkListBox *timeline_list;
} GwTimelineListPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GwTimelineList, gw_timeline_list, GTK_TYPE_BOX)

static gboolean
on_web_view_decide_policy (WebKitWebView *web_view,
                           WebKitPolicyDecision *decision,
                           WebKitPolicyDecisionType decision_type,
                           gpointer user_data)
{
    const gchar *requested_uri;
    const gchar *fragment;
    const gchar *payload;
    const gchar *query;
    gchar *access_token = NULL;
    gchar *code = NULL;
    GError *error = NULL;
    GHashTable *key_value_pairs;
    gsize payload_length;
    guint status_code;
    OAuth2Proxy *proxy = OAUTH2_PROXY (user_data);
    RestProxy *token_proxy;
    RestProxyCall *token_call;
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
        GError *tokens_error = NULL;
        GSettings *settings;
        JsonParser *parser;
        JsonObject *object;

        token_proxy = rest_proxy_new ("https://api.weibo.com/oauth2/access_token",
                                      FALSE);
        token_call = rest_proxy_new_call (token_proxy);

        rest_proxy_call_set_method (token_call, "POST");
        rest_proxy_call_add_header (token_call, "Content-Type",
                                    "application/x-www-form-urlencoded");
        rest_proxy_call_add_param (token_call, "client_id", "1450991920");
        rest_proxy_call_add_param (token_call, "client_secret",
                                   "24afe70740825258ca104ee54acf9712");
        rest_proxy_call_add_param (token_call, "grant_type", "authorization_code");
        rest_proxy_call_add_param (token_call, "redirect_uri",
                                   "https://api.weibo.com/oauth2/default.html");
        rest_proxy_call_add_param (token_call, "code", code);

        rest_proxy_call_sync (token_call, &error);
        if (error != NULL)
        {
            g_error ("Cannot make call: %s", error->message);
            g_error_free (error);
        }

        status_code = rest_proxy_call_get_status_code (token_call);
        if (status_code != 200)
        {
            g_error ("Expected status 200 when requesting access token, instead got status %d (%s)",
                     status_code,
                     rest_proxy_call_get_status_message (token_call));
            g_error_free (error);
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
            g_object_unref (parser);
        }

        object = json_node_get_object (json_parser_get_root (parser));
        if (!json_object_has_member (object, "access_token"))
        {
            g_warning ("Did not find access_token in JSON data");
            g_object_unref (object);
        }

        /* Got the access token */
        access_token = g_strdup (json_object_get_string_member (object, "access_token"));
        oauth2_proxy_set_access_token (proxy, access_token);

        settings = g_settings_new ("org.gnome.Weibo");
        g_settings_set_string (settings, "access-token", access_token);
        g_object_unref (settings);

        goto default_behaviour;
    }

ignore_request:
    webkit_policy_decision_ignore (decision);

    g_free (access_token);
    g_free (code);

    return TRUE;

default_behaviour:
    g_free (access_token);
    g_free (code);

    return FALSE;
}

static void
on_login_button_clicked (void)
{
    gchar *uri;
    GtkWidget *content_area;
    GtkWidget *dialog;
    GtkWidget *web_view;
    RestProxy *proxy;

    web_view = webkit_web_view_new ();
    gtk_widget_set_hexpand (web_view, TRUE);
    gtk_widget_set_vexpand (web_view, TRUE);
    gtk_widget_set_size_request (web_view, 600, 400);

    dialog = gtk_dialog_new ();
    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (gtk_widget_destroy), dialog);
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    gtk_container_add (GTK_CONTAINER (content_area), web_view);

    proxy = oauth2_proxy_new ("1450991920",
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
}

static GtkWidget *
gw_timeline_list_create_placeholder (void)
{
    GtkWidget *box;
    GtkWidget *button;
    GtkWidget *label;
    gchar *markup;

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(box, TRUE);
    gtk_widget_set_vexpand(box, TRUE);

    button = gtk_button_new_with_label ("Login");
    g_signal_connect (button, "clicked",
                      G_CALLBACK (on_login_button_clicked), NULL);
    gtk_widget_set_halign (button, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (box), button, FALSE, TRUE, 0);

    label = gtk_label_new(NULL);
    markup = g_markup_printf_escaped("<big>%s</big>", "Click the button to log in.");
    gtk_label_set_markup (GTK_LABEL (label), markup);
    gtk_box_pack_end (GTK_BOX (box), label, TRUE, TRUE, 0);

    gtk_widget_show_all (box);

    g_free (markup);

    return box;
}

static void
gw_timeline_list_class_init (GwTimelineListClass *klass)
{
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/org/gnome/Weibo/gw-timeline-list.ui");
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  GwTimelineList, timeline_list);
}

static void
gw_timeline_list_init (GwTimelineList *self)
{
    GwTimelineListPrivate *priv;
    GtkWidget *placeholder;

    priv = gw_timeline_list_get_instance_private (self);
    placeholder = gw_timeline_list_create_placeholder ();

    gtk_widget_init_template (GTK_WIDGET (self));

    gtk_list_box_set_placeholder (priv->timeline_list, placeholder);
}

/**
 * gw_timeline_list_new:
 *
 * Create a new #GwTimelineList.
 *
 * Returns: (transfer full): a newly created #GwTimelineList
 */
GwTimelineList *
gw_timeline_list_new (void)
{
    return g_object_new (GW_TYPE_TIMELINE_LIST, NULL);
}
