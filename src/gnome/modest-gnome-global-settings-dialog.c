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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /*HAVE_CONFIG_H*/

#include <glib/gi18n.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtktable.h>
#include <gtk/gtkspinbutton.h>
#include "widgets/modest-global-settings-dialog-priv.h"
#include "widgets/modest-combo-box.h"
#include "gnome/modest-gnome-global-settings-dialog.h"
#include "widgets/modest-ui-constants.h"


/* include other impl specific header files */

/* 'private'/'protected' functions */
static void modest_gnome_global_settings_dialog_class_init (ModestGnomeGlobalSettingsDialogClass *klass);
static void modest_gnome_global_settings_dialog_init       (ModestGnomeGlobalSettingsDialog *obj);
static void modest_gnome_global_settings_dialog_finalize   (GObject *obj);

/* list my signals  */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};

static GtkWidget* create_updating_page  (ModestGnomeGlobalSettingsDialog *self);
static GtkWidget* create_composing_page (ModestGnomeGlobalSettingsDialog *self);

typedef struct _ModestGnomeGlobalSettingsDialogPrivate ModestGnomeGlobalSettingsDialogPrivate;
struct _ModestGnomeGlobalSettingsDialogPrivate {
};
#define MODEST_GNOME_GLOBAL_SETTINGS_DIALOG_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                                           MODEST_TYPE_GNOME_GLOBAL_SETTINGS_DIALOG, \
                                                           ModestGnomeGlobalSettingsDialogPrivate))
/* globals */
static GtkDialogClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

GType
modest_gnome_global_settings_dialog_get_type (void)
{
	static GType my_type = 0;
	if (!my_type) {
		static const GTypeInfo my_info = {
			sizeof(ModestGnomeGlobalSettingsDialogClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) modest_gnome_global_settings_dialog_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(ModestGnomeGlobalSettingsDialog),
			1,		/* n_preallocs */
			(GInstanceInitFunc) modest_gnome_global_settings_dialog_init,
			NULL
		};
		my_type = g_type_register_static (MODEST_TYPE_GLOBAL_SETTINGS_DIALOG,
		                                  "ModestGnomeGlobalSettingsDialog",
		                                  &my_info, 0);
	}
	return my_type;
}

static void
modest_gnome_global_settings_dialog_class_init (ModestGnomeGlobalSettingsDialogClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = modest_gnome_global_settings_dialog_finalize;

	g_type_class_add_private (gobject_class, sizeof(ModestGnomeGlobalSettingsDialogPrivate));
}

static void
modest_gnome_global_settings_dialog_init (ModestGnomeGlobalSettingsDialog *self)
{
	ModestGlobalSettingsDialogPrivate *ppriv;
/* 	GdkGeometry *geometry; */

	ppriv = MODEST_GLOBAL_SETTINGS_DIALOG_GET_PRIVATE (self);

	ppriv->updating_page = create_updating_page (self);
	ppriv->composing_page = create_composing_page (self);
    
	/* Add the notebook pages: */
	gtk_notebook_append_page (GTK_NOTEBOOK (ppriv->notebook), ppriv->updating_page, 
		gtk_label_new (_("mcen_ti_options_updating")));
	gtk_notebook_append_page (GTK_NOTEBOOK (ppriv->notebook), ppriv->composing_page, 
		gtk_label_new (_("mcen_ti_options_composing")));
		
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (self)->vbox), ppriv->notebook);
	gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (self)->vbox), MODEST_MARGIN_HALF);
	gtk_widget_show_all (ppriv->notebook);
}

static void
modest_gnome_global_settings_dialog_finalize (GObject *obj)
{
/* 	free/unref instance resources here */
	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

GtkWidget*
modest_gnome_global_settings_dialog_new (void)
{
	return GTK_WIDGET(g_object_new(MODEST_TYPE_GNOME_GLOBAL_SETTINGS_DIALOG, NULL));
}


/* 
 * Adds the two widgets to a new row in the table
 */
static void
add_to_table (GtkTable *table,
	      GtkWidget *left,
	      GtkWidget *right)
{
	guint n_rows = 0;

	g_object_get (G_OBJECT (table), "n-rows", &n_rows,NULL);

	/* Attach label and value */
	gtk_table_attach (table, 
			  left, 0, 1, 
			  n_rows, n_rows + 1, 
			  GTK_SHRINK|GTK_FILL, 
			  GTK_SHRINK|GTK_FILL, 
			  0, 0);
	gtk_table_attach (table, 
			  right, 1, 2, 
			  n_rows, n_rows + 1, 
			  GTK_EXPAND|GTK_FILL, 
			  GTK_SHRINK|GTK_FILL, 
			  0, 0);
}

/* 
 * We need this because the translations are comming without ":" 
 */
static GtkWidget *
create_label (const gchar *text)
{
	gchar *label_name;
	GtkWidget *label;

	label_name = g_strdup_printf ("%s:", text);
	label = gtk_label_new (label_name);
	g_free (label_name);

	return label;
}

/*
 * Creates the updating page
 */
static GtkWidget*
create_updating_page (ModestGnomeGlobalSettingsDialog *self)
{
	GtkWidget *vbox, *table_update, *table_limit;
	GtkWidget *label, *check, *combo, *spin;
	ModestPairList *list;

	vbox = gtk_vbox_new (FALSE, MODEST_MARGIN_DEFAULT);
	table_update = gtk_table_new (3, 2, FALSE);
	table_limit = gtk_table_new (2, 2, FALSE);
	/* FIXME: set proper values (HIG) */
	gtk_table_set_row_spacings (GTK_TABLE (table_update), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table_update), 12);
	gtk_table_set_row_spacings (GTK_TABLE (table_limit), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table_limit), 12);

	/* Autoupdate */
	label = create_label (_("mcen_fi_options_autoupdate"));
	check = gtk_check_button_new ();
	add_to_table (GTK_TABLE (table_update), label, check);

	/* Connected via */
	label = create_label (_("mcen_fi_options_connectiontype"));
	list = _modest_global_settings_dialog_get_connected_via ();
	combo = modest_combo_box_new (list, g_int_equal);
	modest_pair_list_free (list);
	add_to_table (GTK_TABLE (table_update), label, combo);

	/* Update interval */
	label = create_label (_("mcen_fi_options_updateinterval"));
	list = _modest_global_settings_dialog_get_update_interval ();
	combo = modest_combo_box_new (list, g_int_equal);
	modest_pair_list_free (list);
	add_to_table (GTK_TABLE (table_update), label, combo);

	/* Add to vbox */
	gtk_box_pack_start (GTK_BOX (vbox), table_update, FALSE, FALSE, MODEST_MARGIN_HALF);

	/* Separator */
	gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new (), FALSE, FALSE, MODEST_MARGIN_HALF);

	/* Limits */
	label = create_label (_("mcen_fi_advsetup_sizelimit"));
	spin = gtk_spin_button_new (GTK_ADJUSTMENT (gtk_adjustment_new (1000, 1, 5000, 1, 1, 16)), 
				    1, 0);
	add_to_table (GTK_TABLE (table_limit), label, spin);

	label = create_label (_("mcen_fi_options_playsound"));
	check = gtk_check_button_new ();
	add_to_table (GTK_TABLE (table_limit), label, check);

	/* Add to vbox */
	gtk_box_pack_start (GTK_BOX (vbox), table_limit, FALSE, FALSE, MODEST_MARGIN_HALF);

	return vbox;
}

/*
 * Creates the composing page
 */
static GtkWidget* 
create_composing_page (ModestGnomeGlobalSettingsDialog *self)
{
	GtkWidget *vbox, *table;
	GtkWidget *label, *check, *combo;
	ModestPairList *list;

	vbox = gtk_vbox_new (FALSE, MODEST_MARGIN_DEFAULT);
	table = gtk_table_new (2, 2, FALSE);
	/* FIXME: set proper values */
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);

	/* Update interval */
	label = create_label (_("mcen_fi_options_messageformat"));
	list = _modest_global_settings_dialog_get_msg_formats ();
	combo = modest_combo_box_new (list, g_int_equal);
	modest_pair_list_free (list);
	add_to_table (GTK_TABLE (table), label, combo);

	label = create_label (_("mcen_va_options_include_original_inreply"));
	check = gtk_check_button_new ();
	add_to_table (GTK_TABLE (table), label, check);

	/* Add to vbox */
	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, MODEST_MARGIN_HALF);

	return vbox;
}
