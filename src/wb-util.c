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

#include <glib.h>
#include <json-glib/json-glib.h>

#include "wb-timeline-list.h"
#include "wb-util.h"

/**
 * wb_util_format_time_string:
 * @time: the time string fetched from Weibo API
 *
 * This function parses the time string fetched from Weibo
 * API and return a meaningful time.
 *
 * Returns: A newly allocated string
 */
gchar *
wb_util_format_time_string (const gchar *time)
{
    const gchar *month_str[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "July", "Aug", "Sep", "Oct", "Nov", "Dec"};
    gchar *ret;
    gchar *year;
    gchar *day;
    gboolean same_year;
    gboolean same_month;
    gboolean same_day;
    gchar **vector;
    GDateTime *now;

    g_return_val_if_fail (time != NULL, NULL);

    now = g_date_time_new_now_local ();
    vector = g_strsplit_set (time, " :", -1);
    year = g_strdup_printf ("%d", g_date_time_get_year (now));
    day = g_strdup_printf ("%d", g_date_time_get_day_of_month (now));

    same_year = !g_strcmp0 (year, vector[7]);
    same_month = !g_strcmp0 (month_str[g_date_time_get_month (now) - 1], vector[1]);
    same_day = !g_strcmp0 (day, vector[2]);

    if (same_year && same_month && same_day)
    {
        ret = g_strdup_printf ("%s:%s", vector[3], vector[4]);
    }
    else if ((same_year && same_month)
             || (same_year && !same_month))
    {
        ret = g_strdup_printf ("%s:%s %s %s",
                               vector[3], vector[4], vector[1], vector[2]);
    }
    else
    {
        ret = g_strdup_printf ("%s:%s %s %s %s",
                               vector[3], vector[4], vector[1], vector[2], vector[7]);
    }

    g_date_time_unref (now);
    g_free (year);
    g_free (day);
    g_strfreev (vector);

    return ret;
}

/**
 * wb_util_format_source_string:
 * @source: the source string fetched from Weibo API
 *
 * This function parses a <a> tag and returns the text.
 *
 * Returns: A newly allocated string
 */
gchar *
wb_util_format_source_string (const gchar *source)
{
    gchar **vector;
    gchar *ret;

    g_return_val_if_fail (source != NULL, NULL);

    /* Don't parse it when @source is an empty string */
    if (g_strcmp0 (source, "") == 0)
    {
        return NULL;
    }

    vector = g_strsplit_set (source, "<>", -1);
    ret = g_strdup (vector[2]);

    g_strfreev (vector);

    return ret;
}

/**
 * wb_util_thumbnail_to_original:
 * @thumbnail: the thumbnail uri string fetched from Weibo API
 *
 * This function converts a thumbnail uri string to the
 * original image uri string.
 *
 * Returns: A newly allocated uri string
 */
gchar *
wb_util_thumbnail_to_original (const gchar *thumbnail)
{
    gchar *ret;
    gchar *str;
    g_autofree gchar *part1 = NULL;

    g_return_val_if_fail (thumbnail != NULL, NULL);

    str = g_strrstr (thumbnail, "thumbnail");
    part1 = g_strndup (thumbnail, str - thumbnail);

    ret = g_strconcat (part1, "large", str + 9, NULL);

    return ret;
}

static void
parse_pic_uri (JsonArray *array,
               guint index,
               JsonNode *element_node,
               gpointer user_data)
{
    const gchar *thumbnail;
    gchar *uri;
    WbPostItem *post_item = user_data;
    JsonObject *object;

    object = json_node_get_object (element_node);

    thumbnail = json_object_get_string_member (object, "thumbnail_pic");
    uri = wb_util_thumbnail_to_original (thumbnail);
    g_array_append_val (post_item->picuri_array, uri);
}

void
wb_util_parse_weibo_post (JsonObject *object,
                          WbPostItem *post_item)
{
    JsonArray *pic_array;
    JsonObject *user_object;
    WbUser *user;

    post_item->created_at = g_strdup (json_object_get_string_member (object, "created_at"));
    post_item->id = json_object_get_int_member (object, "id");
    post_item->mid = json_object_get_int_member (object, "mid");
    post_item->idstr = g_strdup (json_object_get_string_member (object, "idstr"));
    post_item->text = g_strdup (json_object_get_string_member (object, "text"));
    post_item->source = g_strdup (json_object_get_string_member (object, "source"));
    post_item->favourited = json_object_get_boolean_member (object, "favorited");
    if (json_object_has_member (object, "thumbnail_pic"))
    {
        post_item->thumbnail_pic = g_strdup (json_object_get_string_member (object,
                                                                            "thumbnail_pic"));
        post_item->bmiddle_pic = g_strdup (json_object_get_string_member (object,
                                                                          "bmiddle_pic"));
    }
    post_item->reposts_count = json_object_get_int_member (object, "reposts_count");
    post_item->comments_count = json_object_get_int_member (object, "comments_count");
    post_item->attitudes_count = json_object_get_int_member (object, "attitudes_count");

    /* Parse the uri for each picture if there is any */
    post_item->picuri_array = g_array_new (FALSE, FALSE, sizeof (gchar *));
    pic_array = json_object_get_array_member (object, "pic_urls");
    if (json_array_get_length (pic_array) != 0)
    {
        json_array_foreach_element (pic_array, parse_pic_uri, post_item);
    }

    user_object = json_object_get_object_member (object, "user");
    user = post_item->user;
    user->id = json_object_get_int_member (user_object, "id");
    user->idstr = g_strdup (json_object_get_string_member (user_object, "idstr"));
    user->name = g_strdup (json_object_get_string_member (user_object, "name"));
    user->nickname = g_strdup (json_object_get_string_member (user_object, "remark"));
    user->location = g_strdup (json_object_get_string_member (user_object, "location"));
    user->profile_image_url = g_strdup (json_object_get_string_member (user_object,
                                                                      "profile_image_url"));
}

GtkWidget *
wb_util_scale_image (GdkPixbuf *pixbuf,
                     gint *width,
                     gint *height)
{
    GtkWidget *image;

    if (*width > MAX_WIDTH && *height > MAX_HEIGHT)
    {
        gdouble scale;
        GdkPixbuf *scaled_pixbuf;

        scale = (gdouble) MAX_WIDTH / *width;
        if (*height * scale > MAX_HEIGHT)
        {
            scale = (gdouble) MAX_HEIGHT / *height;
        }

        scaled_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
                                                 *width * scale,
                                                 *height * scale,
                                                 GDK_INTERP_BILINEAR);
        *width *= scale;
        *height *= scale;

        image = gtk_image_new_from_pixbuf (scaled_pixbuf);

        g_object_unref (scaled_pixbuf);
    }
    else
    {
        image = gtk_image_new_from_pixbuf (pixbuf);
    }

    return image;
}
