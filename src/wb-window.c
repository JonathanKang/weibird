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

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <rest/oauth2-proxy.h>
#include <webkit2/webkit2.h>

#include "wb-headerbar.h"
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
    GtkStack *main_stack;
    GtkWidget *login_box;
    GtkWidget *headerbar;
    GtkWidget *timeline;
} WbWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbWindow, wb_window, GTK_TYPE_APPLICATION_WINDOW)

static const gchar SETTINGS_SCHEMA[] = "com.jonathankang.Weibird";
static const gchar ACCESS_TOKEN[] = "access-token";
static const gchar EXPIRES_IN[] = "expires-in";
static const gchar UID[] = "uid";

static gboolean
on_web_view_decide_policy (WebKitWebView *web_view,
                           WebKitPolicyDecision *decision,
                           WebKitPolicyDecisionType decision_type,
                           gpointer user_data)
{
    const gchar *requested_uri;
    const gchar *fragment;
    const gchar *query;
    gchar *access_token = NULL;
    gchar *code = NULL;
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

        token_proxy = rest_proxy_new ("https://api.weibo.com/oauth2/access_token",
                                      FALSE);
        token_call = rest_proxy_new_call (token_proxy);

        rest_proxy_call_set_method (token_call, "POST");
        rest_proxy_call_add_header (token_call, "Content-Type",
                                    "application/x-www-form-urlencoded");
        rest_proxy_call_add_param (token_call, "client_id", APP_KEY);
        rest_proxy_call_add_param (token_call, "client_secret", APP_SECRECT);
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

    g_free (access_token);
    g_free (code);

    return TRUE;

default_behaviour:
    g_free (access_token);
    g_free (code);

    return FALSE;
}

static void
row_activated_cb (GtkListBox *box,
                  GtkListBoxRow *row,
                  gpointer user_data)
{
    WbPostItem *post_item;
    WbTweetRow *tweet_row = WB_TWEET_ROW (row);
    WbTweetDetailPage *detail;
    WbWindow *window = WB_WINDOW (user_data);
    WbWindowPrivate *priv = wb_window_get_instance_private (window);

    post_item = wb_tweet_row_get_post_item (tweet_row);
    detail = wb_tweet_detail_page_new (post_item);

    gtk_stack_add_named (GTK_STACK (priv->main_stack),
                         GTK_WIDGET (detail), "detail");
    gtk_stack_set_visible_child_name (GTK_STACK (priv->main_stack), "detail");
}

static void
on_login_button_clicked (GtkWidget *button,
                         gpointer user_data)
{
    gchar *uri;
    GtkWidget *content_area;
    GtkWidget *dialog;
    GtkWidget *web_view;
    RestProxy *proxy;
    WbWindow *window;
    WbWindowPrivate *priv;
    WbTimelineList *timeline;

    window = WB_WINDOW (user_data);
    priv = wb_window_get_instance_private (window);

    web_view = webkit_web_view_new ();
    gtk_widget_set_hexpand (web_view, TRUE);
    gtk_widget_set_vexpand (web_view, TRUE);
    gtk_widget_set_size_request (web_view, 600, 400);

    dialog = gtk_dialog_new ();
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (gtk_widget_destroy), dialog);
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    gtk_container_add (GTK_CONTAINER (content_area), web_view);

    proxy = oauth2_proxy_new (APP_KEY,
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

    gtk_stack_set_visible_child (priv->main_stack, priv->timeline);
    timeline = WB_TIMELINE_LIST (priv->timeline);
    wb_timeline_list_get_home_timeline (timeline, FALSE);
}

static void
wb_window_init (WbWindow *window)
{
    gchar *access_token;
    GdkScreen *screen;
    GtkCssProvider *provider;
    GtkListBox *listbox;
    GSettings *settings;
    WbTimelineList *list;
    WbWindowPrivate *priv;

    /* Ensure GTK+ private types used by the template definition
     * before calling gtk_widget_init_template()
     */
    g_type_ensure (WB_TYPE_HEADERBAR);
    g_type_ensure (WB_TYPE_TIMELINE_LIST);

    gtk_widget_init_template (GTK_WIDGET (window));

    priv = wb_window_get_instance_private (window);
    list = WB_TIMELINE_LIST (priv->timeline);

    listbox = wb_timeline_list_get_listbox (list);
    g_signal_connect (listbox, "row-activated", G_CALLBACK (row_activated_cb), window);

    settings = g_settings_new (SETTINGS_SCHEMA);
    access_token = g_settings_get_string (settings, ACCESS_TOKEN);
    if (g_strcmp0 (access_token, "") != 0)
    {
        gtk_stack_set_visible_child (priv->main_stack, priv->timeline);
        wb_timeline_list_get_home_timeline (list, FALSE);
    }
    else
    {
        gtk_stack_set_visible_child (priv->main_stack, priv->login_box);
    }

    screen = gdk_screen_get_default ();
    provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_resource (provider, "/com/jonathankang/Weibird/wb-style.css");
    gtk_style_context_add_provider_for_screen (screen,
                                               GTK_STYLE_PROVIDER (provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_free (access_token);
    g_object_unref (provider);
    g_object_unref (settings);
}
static void
wb_window_class_init (WbWindowClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/com/jonathankang/Weibird/wb-window.ui");
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbWindow, main_stack);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbWindow, login_box);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbWindow, headerbar);
    gtk_widget_class_bind_template_child_private (widget_class,
                                                  WbWindow, timeline);

    gtk_widget_class_bind_template_callback (widget_class, on_login_button_clicked);
}

GtkWidget *
wb_window_new (GtkApplication *application)
{
    return g_object_new (WB_TYPE_WINDOW,
                         "application", application, NULL);
}
