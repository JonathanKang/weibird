/*
 *  Weibird - View and compose weibo
 *  Copyright (C) 2018 Jonathan Kang <jonathankang@gnome.org>.
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

#include "wb-comment-list.h"

struct _WbCommentList
{
		GtkListBox parent_instance;
};

G_DEFINE_TYPE (WbCommentList, wb_comment_list, GTK_TYPE_LIST_BOX)

static void
listbox_update_header_func (GtkListBoxRow *row,
                            GtkListBoxRow *before,
                            gpointer user_data)
{
    GtkWidget *header;

    if (before == NULL)
    {
        gtk_list_box_row_set_header (row, NULL);
        return;
    }

    header = gtk_list_box_row_get_header (row);
    if (header == NULL)
    {
        header = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_show (header);
        gtk_list_box_row_set_header (row, header);
    }
}

static void
wb_comment_list_class_init (WbCommentListClass *klass)
{
}

static void
wb_comment_list_init (WbCommentList *self)
{
    gtk_list_box_set_header_func (GTK_LIST_BOX (self),
                                  (GtkListBoxUpdateHeaderFunc) listbox_update_header_func,
                                  NULL, NULL);
    gtk_list_box_set_selection_mode (GTK_LIST_BOX (self), GTK_SELECTION_NONE);
}

/**
 * wb_comment_list_new:
 *
 * Create a new #WbCommentList.
 *
 * Returns: (transfer full): a newly created #WbCommentList
 */
WbCommentList *
wb_comment_list_new (void)
{
    return g_object_new (WB_TYPE_COMMENT_LIST, NULL);
}
