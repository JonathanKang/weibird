/*
 *  GNOME Weibo - view and compose weibo
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

#include "gw-image.h"
#include "gw-multi-media-widget.h"

enum
{
    PROP_0,
    PROP_N_CHILDS,
    PROP_PIC_URIS,
    N_PROPS
};

struct _GwMultiMediaWidget
{
    /*< private >*/
    GtkGrid parent_instance;
};

typedef struct
{
    GArray *pic_uris;
    gint n_childs;
} GwMultiMediaWidgetPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GwMultiMediaWidget, gw_multi_media_widget, GTK_TYPE_GRID)

static GParamSpec *obj_properties [N_PROPS] = { NULL, };

static void
gw_multi_media_widget_constructed (GObject *object)
{
    gint i;
    gint left, top;
    GtkWidget *child;
    GwMultiMediaWidget *self;
    GwMultiMediaWidgetPrivate *priv;

    self = GW_MULTI_MEDIA_WIDGET (object);
    priv = gw_multi_media_widget_get_instance_private (self);

    for (i = 0; i < priv->n_childs; i++)
    {
        child = gw_image_new (g_array_index (priv->pic_uris, gchar *, i));

        if (i < 3)
        {
            top = 0;
            left = i;
        }
        else if (i >= 3 && i < 6)
        {
            top = 1;
            left = i - 3;
        }
        else
        {
            top = 2;
            left = i - 6;
        }
        gtk_grid_attach (GTK_GRID (self), child, left, top, 1, 1);
    }

    G_OBJECT_CLASS (gw_multi_media_widget_parent_class)->constructed (object);
}

static void
gw_multi_media_widget_finalize (GObject *object)
{
    G_OBJECT_CLASS (gw_multi_media_widget_parent_class)->finalize (object);
}

static void
gw_multi_media_widget_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
    GwMultiMediaWidget *self;
    GwMultiMediaWidgetPrivate *priv;

    self = GW_MULTI_MEDIA_WIDGET (object);
    priv = gw_multi_media_widget_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_N_CHILDS:
            g_value_set_int (value, priv->n_childs);
            break;
        case PROP_PIC_URIS:
            g_value_set_pointer (value, priv->pic_uris);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gw_multi_media_widget_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
    GwMultiMediaWidget *self;
    GwMultiMediaWidgetPrivate *priv;

    self = GW_MULTI_MEDIA_WIDGET (object);
    priv = gw_multi_media_widget_get_instance_private (self);

		switch (prop_id)
    {
        case PROP_N_CHILDS:
            priv->n_childs = g_value_get_int (value);
            break;
        case PROP_PIC_URIS:
            priv->pic_uris = g_value_get_pointer (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gw_multi_media_widget_class_init(GwMultiMediaWidgetClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = gw_multi_media_widget_constructed;
    object_class->finalize = gw_multi_media_widget_finalize;
    object_class->get_property = gw_multi_media_widget_get_property;
    object_class->set_property = gw_multi_media_widget_set_property;

    obj_properties[PROP_N_CHILDS] = g_param_spec_int ("n-childs",
                                                      "Child Number",
                                                      "Number of child widgets",
                                                      1, 9, 1,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT_ONLY);
    obj_properties[PROP_PIC_URIS] = g_param_spec_pointer ("pic-uris",
                                                          "Picture URIs",
                                                          "URI for pictures",
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS);
    g_object_class_install_properties (object_class, N_PROPS, obj_properties);
}

static void
gw_multi_media_widget_init (GwMultiMediaWidget *self)
{
}

/**
 * gw_multi_media_widget_new:
 *
 * Create a new #GwMultiMediaWidget.
 *
 * Returns: (transfer full): a newly created #GwMultiMediaWidget
 */
GtkWidget *
gw_multi_media_widget_new (gint n_childs, const GArray *pic_uris)
{
    return g_object_new (GW_TYPE_MULTI_MEDIA_WIDGET,
                         "n-childs", n_childs,
                         "pic-uris", pic_uris,
                         NULL);
}
