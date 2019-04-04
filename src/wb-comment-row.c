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

#include <glib.h>
#include <gtk/gtk.h>

#include "wb-comment.h"
#include "wb-comment-row.h"
#include "wb-image-button.h"
#include "wb-util.h"

enum {
      PROP_0,
      PROP_COMMENT,
      N_PROPS
};

struct _WbCommentRow
{
		GtkListBoxRow parent_instance;
};

typedef struct
{
    gint reply_count;
    GtkWidget *grid;
    WbComment *comment;
} WbCommentRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WbCommentRow, wb_comment_row, GTK_TYPE_LIST_BOX_ROW)

static GParamSpec *obj_properties [N_PROPS];

WbComment *
wb_comment_row_get_comment (WbCommentRow *self)
{
    WbCommentRowPrivate *priv;

    priv = wb_comment_row_get_instance_private (self);

    return priv->comment;
}

void
wb_comment_row_insert_reply (WbCommentRow *self,
                             WbComment *comment)
{
    GtkStyleContext *context;
    GtkWidget *comment_label;
    GtkWidget *name_label;
    GtkWidget *hbox;
    WbCommentRowPrivate *priv;

    g_return_if_fail (WB_COMMENT_ROW (self));

    priv = wb_comment_row_get_instance_private (self);

    priv->reply_count++;

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
    context = gtk_widget_get_style_context (hbox);
    gtk_style_context_add_class (context, "reply-comments");

    if (g_strcmp0 (comment->user->nickname, "") != 0)
    {
        name_label = gtk_label_new (comment->user->nickname);
    }
    else
    {
        name_label = gtk_label_new (comment->user->name);
    }
    gtk_widget_set_halign (name_label, GTK_ALIGN_START);
    gtk_widget_set_valign (name_label, GTK_ALIGN_START);
    gtk_box_pack_start (GTK_BOX (hbox), name_label, FALSE, FALSE, 0);

    comment_label = gtk_label_new (comment->text);
    gtk_widget_set_halign (comment_label, GTK_ALIGN_START);
    gtk_label_set_line_wrap (GTK_LABEL (comment_label), TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), comment_label, FALSE, FALSE, 0);
    gtk_widget_show_all (hbox);

    gtk_grid_attach (GTK_GRID (priv->grid), hbox,
                     1, priv->reply_count + 1, 2, 1);
}

static void
wb_comment_row_constructed (GObject *object)
{
    gchar *created_at;
    GtkStyleContext *context;
    GtkWidget *comment_label;
    GtkWidget *name_label;
    GtkWidget *time_label;
    WbImageButton *avatar;
    WbCommentRow *self = WB_COMMENT_ROW (object);
    WbCommentRowPrivate *priv = wb_comment_row_get_instance_private (self);

    avatar = wb_image_button_new (WB_MEDIA_TYPE_AVATAR,
                                  priv->comment->user->profile_image_url,
                                  1, 50, 50);
    gtk_grid_attach (GTK_GRID (priv->grid), GTK_WIDGET (avatar), 0, 0, 1, 1);

    if (g_strcmp0 (priv->comment->user->nickname, "") != 0)
    {
        name_label = gtk_label_new (priv->comment->user->nickname);
    }
    else
    {
        name_label = gtk_label_new (priv->comment->user->name);
    }
    gtk_widget_set_halign (name_label, GTK_ALIGN_START);
    gtk_grid_attach (GTK_GRID (priv->grid), GTK_WIDGET (name_label), 1, 0, 1, 1);

    created_at = wb_util_format_time_string (priv->comment->created_at);
    time_label = gtk_label_new (created_at);
    context = gtk_widget_get_style_context (time_label);
    gtk_style_context_add_class (context, "dim-label");
    gtk_widget_set_halign (time_label, GTK_ALIGN_END);
    gtk_grid_attach (GTK_GRID (priv->grid), GTK_WIDGET (time_label), 2, 0, 1, 1);

    comment_label = gtk_label_new (priv->comment->text);
    gtk_widget_set_halign (comment_label, GTK_ALIGN_START);
    gtk_widget_set_hexpand (comment_label, TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (comment_label), TRUE);
    gtk_grid_attach (GTK_GRID (priv->grid), GTK_WIDGET (comment_label),
                     1, 1, 2, 1);

    gtk_widget_show_all (GTK_WIDGET (self));

    G_OBJECT_CLASS (wb_comment_row_parent_class)->constructed (object);
}

static void
wb_comment_row_finalize (GObject *object)
{
    WbCommentRow *self = WB_COMMENT_ROW (object);
    WbCommentRowPrivate *priv = wb_comment_row_get_instance_private (self);

    g_object_unref (priv->comment);

    G_OBJECT_CLASS (wb_comment_row_parent_class)->finalize (object);
}

static void
wb_comment_row_get_property (GObject *object,
                             guint prop_id,
                             GValue *value,
                             GParamSpec *pspec)
{
    WbCommentRow *self = WB_COMMENT_ROW (object);
    WbCommentRowPrivate *priv = wb_comment_row_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_COMMENT:
            g_value_set_object (value, priv->comment);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
wb_comment_row_set_property (GObject *object,
                             guint prop_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
    WbCommentRow *self = WB_COMMENT_ROW (object);
    WbCommentRowPrivate *priv = wb_comment_row_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_COMMENT:
            priv->comment = g_value_dup_object (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
wb_comment_row_class_init (WbCommentRowClass *klass)
{
		GObjectClass *object_class = G_OBJECT_CLASS (klass);

		object_class->constructed = wb_comment_row_constructed;
		object_class->finalize = wb_comment_row_finalize;
		object_class->get_property = wb_comment_row_get_property;
		object_class->set_property = wb_comment_row_set_property;

    obj_properties[PROP_COMMENT] = g_param_spec_object ("comment",
                                                        "Comment",
                                                        "Comment data",
                                                        WB_TYPE_COMMENT,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS);
    g_object_class_install_properties (object_class, N_PROPS, obj_properties);
}

static void
wb_comment_row_init (WbCommentRow *self)
{
    WbCommentRowPrivate *priv;

    priv = wb_comment_row_get_instance_private (self);

    priv->reply_count = 0;

    priv->grid = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (priv->grid), 6);
    gtk_grid_set_row_spacing (GTK_GRID (priv->grid), 6);
    gtk_widget_set_margin_start (GTK_WIDGET (priv->grid), 12);
    gtk_widget_set_margin_end (GTK_WIDGET (priv->grid), 12);
    gtk_widget_set_margin_top (GTK_WIDGET (priv->grid), 6);
    gtk_widget_set_margin_bottom (GTK_WIDGET (priv->grid), 6);
    gtk_container_add (GTK_CONTAINER (self), priv->grid);
}

/**
 * wb_comment_row_new:
 *
 * Create a new #WbCommentRow.
 *
 * Returns: (transfer full): a newly created #WbCommentRow
 */
WbCommentRow *
wb_comment_row_new (WbComment *comment)
{
    return g_object_new (WB_TYPE_COMMENT_ROW,
                         "comment", comment,
                         NULL);
}
