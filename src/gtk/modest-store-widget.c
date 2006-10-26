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

#include "modest-store-widget.h"
#include <string.h>

/* 'private'/'protected' functions */
static void modest_store_widget_class_init (ModestStoreWidgetClass *klass);
static void modest_store_widget_init       (ModestStoreWidget *obj);
static void modest_store_widget_finalize   (GObject *obj);
/* list my signals  */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};

typedef struct _ModestStoreWidgetPrivate ModestStoreWidgetPrivate;
struct _ModestStoreWidgetPrivate {
	
	gchar* proto;
	
	GtkWidget *servername;
	GtkWidget *username;
	GtkWidget *security;
	GtkWidget *auth;
	GtkWidget *chooser;
	GtkWidget *remember_pwd;
	
	ModestWidgetFactory *factory;
};
#define MODEST_STORE_WIDGET_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                                 MODEST_TYPE_STORE_WIDGET, \
                                                 ModestStoreWidgetPrivate))
/* globals */
static GtkContainerClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

GType
modest_store_widget_get_type (void)
{
	static GType my_type = 0;
	if (!my_type) {
		static const GTypeInfo my_info = {
			sizeof(ModestStoreWidgetClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) modest_store_widget_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(ModestStoreWidget),
			1,		/* n_preallocs */
			(GInstanceInitFunc) modest_store_widget_init,
			NULL
		};
		my_type = g_type_register_static (GTK_TYPE_VBOX,
		                                  "ModestStoreWidget",
		                                  &my_info, 0);
	}
	return my_type;
}

static void
modest_store_widget_class_init (ModestStoreWidgetClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = modest_store_widget_finalize;

	g_type_class_add_private (gobject_class, sizeof(ModestStoreWidgetPrivate));

	/* signal definitions go here, e.g.: */
/* 	signals[MY_SIGNAL_1] = */
/* 		g_signal_new ("my_signal_1",....); */
/* 	signals[MY_SIGNAL_2] = */
/* 		g_signal_new ("my_signal_2",....); */
/* 	etc. */
}

static void
modest_store_widget_init (ModestStoreWidget *obj)
{
 	ModestStoreWidgetPrivate *priv;
	
	priv = MODEST_STORE_WIDGET_GET_PRIVATE(obj); 

	priv->proto = NULL;
}



static GtkWidget *
maildir_configuration (ModestStoreWidget *self)
{
	ModestStoreWidgetPrivate *priv;
	GtkWidget *label, *box, *hbox;
	
	priv = MODEST_STORE_WIDGET_GET_PRIVATE(self);
	box = gtk_vbox_new (FALSE, 6);
	
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label),
			      _("<b>Maildir configuration</b>"));	
	gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 6);
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label),
			      _("Please select the path to your Maildir below"));	
	gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 6);
	
	label = gtk_label_new (_("Path:"));

	priv->chooser = 
		gtk_file_chooser_button_new (_("(none)"),
					     GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);

	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 6);
	gtk_box_pack_start (GTK_BOX(hbox), priv->chooser, FALSE, FALSE, 6);

	gtk_box_pack_start (GTK_BOX(box), hbox, FALSE, FALSE, 6);	

	return box;
}


static GtkWidget*
mbox_configuration (ModestStoreWidget *self)
{
	ModestStoreWidgetPrivate *priv;
	GtkWidget *label, *box, *hbox;
	
	priv = MODEST_STORE_WIDGET_GET_PRIVATE(self);
	box = gtk_vbox_new (FALSE, 6);
	
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label),
			      _("<b>Mbox configuration</b>"));	
	gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 6);
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label),
			      _("Please select your mbox file below"));	
	gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 6);
	
	label = gtk_label_new (_("mbox:"));

	priv->chooser =
		gtk_file_chooser_button_new (_("(none)"),
					     GTK_FILE_CHOOSER_ACTION_OPEN);
	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 6);
	gtk_box_pack_start (GTK_BOX(hbox), priv->chooser, FALSE, FALSE, 6);
	
	gtk_box_pack_start (GTK_BOX(box), hbox, FALSE, FALSE, 6);	

	return box;
}


static GtkWidget*
imap_pop_configuration (ModestStoreWidget *self)
{
	ModestStoreWidgetPrivate *priv;
	GtkWidget *label, *box, *hbox;

	priv = MODEST_STORE_WIDGET_GET_PRIVATE(self);
	box = gtk_vbox_new (FALSE, 6);
	
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(label),_("<b>Server configuration</b>"));
	gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 6);
	
	hbox	= gtk_hbox_new (FALSE, 6);
	label   = gtk_label_new (_("Username:"));
 	if (!priv->username)
		priv->username = gtk_entry_new_with_max_length (40);
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(hbox), priv->username,FALSE, FALSE, 6);
	gtk_box_pack_start (GTK_BOX(box), hbox, FALSE, FALSE, 0);	

	hbox	= gtk_hbox_new (FALSE, 6);
	label   = gtk_label_new (_("Server:"));
	if (!priv->servername)
		priv->servername = gtk_entry_new_with_max_length (40);
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(hbox), priv->servername,FALSE, FALSE, 6);
	gtk_box_pack_start (GTK_BOX(box), hbox, FALSE, FALSE, 0);	

	label = gtk_label_new(NULL);
	gtk_label_set_markup (GTK_LABEL(label),_("<b>Security</b>"));
	gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 6);
	label = gtk_label_new(NULL);
	gtk_label_set_text (GTK_LABEL(label),_("Connection type:"));
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(hbox),  modest_widget_factory_get_combo_box
			    (priv->factory, MODEST_COMBO_BOX_TYPE_SECURITY_PROTOS),
			    FALSE, FALSE,0);
	gtk_box_pack_start (GTK_BOX(box), hbox, FALSE, FALSE, 0);
	
	hbox = gtk_hbox_new (FALSE, 6);
	label = gtk_label_new(NULL);

	gtk_label_set_text (GTK_LABEL(label),_("Authentication:"));
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 6);
	gtk_box_pack_start (GTK_BOX(hbox),   modest_widget_factory_get_combo_box
			    (priv->factory, MODEST_COMBO_BOX_TYPE_AUTH_PROTOS),
			    FALSE, FALSE, 0);

	priv->remember_pwd =
		gtk_check_button_new_with_label (_("Remember password"));
	gtk_box_pack_start (GTK_BOX(hbox),priv->remember_pwd,
			    FALSE, FALSE, 0);
	
	gtk_box_pack_start (GTK_BOX(box), hbox, FALSE, FALSE, 0);

	return box;
}


static void
modest_store_widget_finalize (GObject *obj)
{
	ModestStoreWidgetPrivate *priv;
	priv = MODEST_STORE_WIDGET_GET_PRIVATE(obj);
	
	if (priv->factory) {
		g_object_unref (priv->factory);
		priv->factory = NULL;
	}

	if (priv->proto) {
		g_free (priv->proto);
		priv->proto = NULL;
	}

	G_OBJECT_CLASS(parent_class)->finalize (obj);
}



GtkWidget*
modest_store_widget_new (ModestWidgetFactory *factory, const gchar *proto)
{
	GObject *obj;
	GtkWidget *w;
	ModestStoreWidget *self;
	ModestStoreWidgetPrivate *priv;
	
	g_return_val_if_fail (proto, NULL);
	g_return_val_if_fail (factory, NULL);

	obj = g_object_new(MODEST_TYPE_STORE_WIDGET, NULL);
	self = MODEST_STORE_WIDGET(obj);
	priv = MODEST_STORE_WIDGET_GET_PRIVATE(self);

	g_object_ref (factory);
	priv->factory = factory;

	priv->proto = g_strdup (proto);
	
	if (strcmp (proto, MODEST_PROTOCOL_STORE_POP) == 0 ||
	    strcmp (proto, MODEST_PROTOCOL_STORE_IMAP) == 0) {
		w = imap_pop_configuration (self);
	} else if (strcmp (proto, MODEST_PROTOCOL_STORE_MAILDIR) == 0) {
		w = maildir_configuration (self);
	}  else if (strcmp (proto, MODEST_PROTOCOL_STORE_MBOX) == 0) {
		w = mbox_configuration (self);
	} else
		w = gtk_label_new ("");
	
	gtk_widget_show_all (w);
	gtk_box_pack_start (GTK_BOX(self), w, FALSE, FALSE, 2);

	return GTK_WIDGET(self);
}


gboolean
modest_store_widget_get_remember_password (ModestStoreWidget *self)
{
	ModestStoreWidgetPrivate *priv;

	g_return_val_if_fail (self, FALSE);
	priv = MODEST_STORE_WIDGET_GET_PRIVATE(self);

	return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->remember_pwd));
}


const gchar*
modest_store_widget_get_username (ModestStoreWidget *self)
{
	ModestStoreWidgetPrivate *priv;

	g_return_val_if_fail (self, NULL);
	priv = MODEST_STORE_WIDGET_GET_PRIVATE(self);
	
	return gtk_entry_get_text (GTK_ENTRY(priv->username));
}

const gchar*
modest_store_widget_get_servername (ModestStoreWidget *self)
{
	ModestStoreWidgetPrivate *priv;

	g_return_val_if_fail (self, NULL);
	priv = MODEST_STORE_WIDGET_GET_PRIVATE(self);
	
	return gtk_entry_get_text (GTK_ENTRY(priv->servername));
}


const gchar*
modest_store_widget_get_proto (ModestStoreWidget *self)
{
	ModestStoreWidgetPrivate *priv;

	g_return_val_if_fail (self, FALSE);
	priv = MODEST_STORE_WIDGET_GET_PRIVATE(self);

	return priv->proto;
}

