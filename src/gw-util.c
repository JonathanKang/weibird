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

#include "gw-util.h"

gchar *
gw_util_format_time_string (gchar *str)
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

    g_return_val_if_fail (str != NULL, NULL);

    now = g_date_time_new_now_local ();
    vector = g_strsplit_set (str, " :", -1);
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
    g_free (str);
    g_free (year);
    g_free (day);
    g_strfreev (vector);

    return ret;
}
