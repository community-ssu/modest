/* Copyright (c) 2006, Nokia Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of the Nokia Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "modest-combo-box.h"

/* 'private'/'protected' functions */
static void modest_combo_box_class_init (ModestComboBoxClass *klass);
static void modest_combo_box_init       (ModestComboBox *obj);
static void modest_combo_box_finalize   (GObject *obj);
/* list my signals  */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};


enum {
	COLUMN_ID,
	COLUMN_DISPLAY_NAME,
	COLUMN_NUM
};

typedef struct _ModestComboBoxPrivate ModestComboBoxPrivate;
struct _ModestComboBoxPrivate {
	/* my private members go here, eg. */
	/* gboolean frobnicate_mode; */
};
#define MODEST_COMBO_BOX_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                              MODEST_TYPE_COMBO_BOX, \
                                              ModestComboBoxPrivate))
/* globals */
static GObjectClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

GType
modest_combo_box_get_type (void)
{
	static GType my_type = 0;
	if (!my_type) {
		static const GTypeInfo my_info = {
			sizeof(ModestComboBoxClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) modest_combo_box_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(ModestComboBox),
			1,		/* n_preallocs */
			(GInstanceInitFunc) modest_combo_box_init,
			NULL
		};
		my_type = g_type_register_static (G_TYPE_OBJECT,
		                                  "ModestComboBox",
		                                  &my_info, 0);
	}
	return my_type;
}

static void
modest_combo_box_class_init (ModestComboBoxClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = modest_combo_box_finalize;

	g_type_class_add_private (gobject_class, sizeof(ModestComboBoxPrivate));

	/* signal definitions go here, e.g.: */
/* 	signals[MY_SIGNAL_1] = */
/* 		g_signal_new ("my_signal_1",....); */
/* 	signals[MY_SIGNAL_2] = */
/* 		g_signal_new ("my_signal_2",....); */
/* 	etc. */
}

static void
modest_combo_box_init (ModestComboBox *obj)
{
/* uncomment the following if you init any of the private data */
/* 	ModestComboBoxPrivate *priv = MODEST_COMBO_BOX_GET_PRIVATE(obj); */

/* 	initialize this object, eg.: */
/* 	priv->frobnicate_mode = FALSE; */
}

static void
modest_combo_box_finalize (GObject *obj)
{
/* 	free/unref instance resources here */
	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

static GtkTreeModel*
get_model (const GSList *pairs)
{
	GtkTreeIter iter;
	GtkListStore *store;
	const GSList *cursor;
	
	store = gtk_list_store_new (2,
				    G_TYPE_POINTER, /* the id */
				    G_TYPE_STRING); /* the display name */
	cursor = pairs;
	while (cursor) {
		ModestPair *pair = (ModestPair*)cursor->data;
		gtk_list_store_insert_with_values (store, &iter, G_MAXINT,
						   COLUMN_ID,           pair->first,
						   COLUMN_DISPLAY_NAME, pair->second,
						   -1);
		cursor = cursor->next;
	}
	
	return GTK_TREE_MODEL (store);
}


GtkWidget*
modest_combo_box_new (const GSList *pairs)
{
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GObject *obj;

	obj  = G_OBJECT(g_object_new(MODEST_TYPE_COMBO_BOX, NULL));
		
	model = get_model (pairs);

	if (model) {
		gtk_combo_box_set_model (GTK_COMBO_BOX(obj), model);
		g_object_unref (model);
		gtk_cell_layout_clear (GTK_CELL_LAYOUT(obj));
		
		renderer = gtk_cell_renderer_text_new ();
		gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(obj),
					    renderer, FALSE);  
		gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(obj),
						renderer, "text",
						COLUMN_DISPLAY_NAME, NULL); 
	}
	
	return GTK_WIDGET(obj);
}



gpointer
modest_combo_box_get_active_id (ModestComboBox *self)
{
	GtkTreeIter iter;
	gpointer retval;
	
	g_return_val_if_fail (self, NULL);

	if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX(self), &iter))
		retval = NULL; /* nothing found */
	else {
		GtkTreeModel *model;
		GValue val;
		
		model = gtk_combo_box_get_model (GTK_COMBO_BOX(self));

		g_value_init (&val, G_TYPE_POINTER);
		gtk_tree_model_get_value (model, &iter, COLUMN_ID, &val);

		retval = g_value_get_pointer (&val);
		g_value_unset (&val);
	}
	return retval;
}
