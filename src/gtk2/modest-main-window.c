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

#include "modest-main-window.h"
#include "modest-account-view-window.h"
#include "modest-msg-window.h"

/* 'private'/'protected' functions */
static void modest_main_window_class_init    (ModestMainWindowClass *klass);
static void modest_main_window_init          (ModestMainWindow *obj);
static void modest_main_window_finalize      (GObject *obj);

/* list my signals */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};

typedef struct _ModestMainWindowPrivate ModestMainWindowPrivate;
struct _ModestMainWindowPrivate {
	GtkWidget *toolbar;
	GtkWidget *menubar;
	GtkWidget *progress_bar;
	GtkWidget *status_bar;
	GtkWidget *folder_paned;
	GtkWidget *msg_paned;

	ModestWidgetFactory *widget_factory;
	ModestConf *conf;
	
	ModestHeaderView *header_view;
	ModestFolderView *folder_view;
	ModestMsgView    *msg_preview;
};


#define MODEST_MAIN_WINDOW_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                                MODEST_TYPE_MAIN_WINDOW, \
                                                ModestMainWindowPrivate))
/* globals */
static GtkWindowClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

GType
modest_main_window_get_type (void)
{
	static GType my_type = 0;
	if (!my_type) {
		static const GTypeInfo my_info = {
			sizeof(ModestMainWindowClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) modest_main_window_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(ModestMainWindow),
			1,		/* n_preallocs */
			(GInstanceInitFunc) modest_main_window_init,
		};
		my_type = g_type_register_static (GTK_TYPE_WINDOW,
		                                  "ModestMainWindow",
		                                  &my_info, 0);
	}
	return my_type;
}

static void
modest_main_window_class_init (ModestMainWindowClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = modest_main_window_finalize;

	g_type_class_add_private (gobject_class, sizeof(ModestMainWindowPrivate));

	/* signal definitions go here, e.g.: */
/* 	signals[MY_SIGNAL_1] = */
/* 		g_signal_new ("my_signal_1",....); */
/* 	signals[MY_SIGNAL_2] = */
/* 		g_signal_new ("my_signal_2",....); */
/* 	etc. */
}

static void
modest_main_window_init (ModestMainWindow *obj)
{
	ModestMainWindowPrivate *priv;

	priv = MODEST_MAIN_WINDOW_GET_PRIVATE(obj);

	priv->widget_factory = NULL;
}

static void
modest_main_window_finalize (GObject *obj)
{
	ModestMainWindowPrivate *priv;
	
	priv = MODEST_MAIN_WINDOW_GET_PRIVATE(obj);

	if (priv->widget_factory) {
		g_object_unref (G_OBJECT(priv->widget_factory));
		priv->widget_factory = NULL;
	}

	if (priv->conf) {
		g_object_unref (G_OBJECT(priv->conf));
		priv->conf = NULL;
	}		
	G_OBJECT_CLASS(parent_class)->finalize (obj);
}


static void
on_menu_about (GtkWidget *widget, gpointer data)
{
	GtkWidget *about;

	about = gtk_about_dialog_new ();
	gtk_about_dialog_set_name (GTK_ABOUT_DIALOG(about), PACKAGE_NAME);
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about),PACKAGE_VERSION);
	gtk_about_dialog_set_copyright (
		GTK_ABOUT_DIALOG(about),
		_("Copyright (c) 2006, Nokia Corporation\n"
		  "All rights reserved."));
	gtk_about_dialog_set_comments (	GTK_ABOUT_DIALOG(about),
		_("a modest e-mail client"));
	gtk_dialog_run (GTK_DIALOG (about));
	gtk_widget_destroy(about);
}


static void
on_menu_accounts (ModestMainWindow *self, guint action, GtkWidget *widget)
{
	GtkWidget *account_win;
	ModestMainWindowPrivate *priv;

	g_return_if_fail (widget);
	g_return_if_fail (self);
	
	priv        = MODEST_MAIN_WINDOW_GET_PRIVATE(self);	
	account_win = modest_account_view_window_new (priv->conf,
						      priv->widget_factory);
	gtk_widget_show (account_win);
}


static void
on_menu_new_message (ModestMainWindow *self, guint action, GtkWidget *widget)
{
	GtkWidget *msg_win;

	msg_win = modest_msg_window_new (MODEST_MSG_WINDOW_TYPE_NEW,
					 NULL);

	gtk_widget_show (msg_win);
}



/* Our menu, an array of GtkItemFactoryEntry structures that defines each menu item */
static GtkItemFactoryEntry menu_items[] = {
	{ "/_File",		NULL,			NULL,           0, "<Branch>" },
	{ "/File/_New",		"<control>N",		NULL,		0, "<StockItem>", GTK_STOCK_NEW },
	{ "/File/_Open",	"<control>O",		NULL,		0, "<StockItem>", GTK_STOCK_OPEN },
	{ "/File/_Save",	"<control>S",		NULL,		0, "<StockItem>", GTK_STOCK_SAVE },
	{ "/File/Save _As",	NULL,			NULL,           0, "<Item>" },
	{ "/File/sep1",		NULL,			NULL,           0, "<Separator>" },
	{ "/File/_Quit",	"<CTRL>Q",		gtk_main_quit,  0, "<StockItem>", GTK_STOCK_QUIT },

	{ "/_Edit",		NULL,			NULL,           0, "<Branch>" },
	{ "/Edit/_Undo",	"<CTRL>Z",		NULL,		0, "<StockItem>", GTK_STOCK_UNDO },
	{ "/Edit/_Redo",	"<shift><CTRL>Z",	NULL,		0, "<StockItem>", GTK_STOCK_REDO },
	{ "/File/sep1",		NULL,			NULL,           0, "<Separator>" },
	{ "/Edit/Cut",		"<control>X",		NULL,		0, "<StockItem>", GTK_STOCK_CUT  },
	{ "/Edit/Copy",		"<CTRL>C",		NULL,           0, "<StockItem>", GTK_STOCK_COPY },
	{ "/Edit/Paste",	NULL,			NULL,           0, "<StockItem>", GTK_STOCK_PASTE},
	{ "/Edit/sep1",		NULL,			NULL,           0, "<Separator>" },
	{ "/Edit/Delete",	"<CTRL>Q",		NULL,           0, "<Item>" },
	{ "/Edit/Select all",	"<CTRL>A",		NULL,           0, "<Item>" },
	{ "/Edit/Deelect all",  "<Shift><CTRL>A",	NULL,           0, "<Item>" },

	{ "/_Actions",                NULL,		NULL,		0, "<Branch>" },
	{ "/Actions/_New Message",    NULL,		on_menu_new_message,		0, "<Item>" },
	{ "/Actions/_Reply",    NULL,			NULL,		0, "<Item>" },
	{ "/Actions/_Forward",  NULL,			NULL,		0, "<Item>" },
	{ "/Actions/_Bounce",   NULL,			NULL,		0, "<Item>" },	
	
	{ "/_Options",		 NULL,			NULL,		0, "<Branch>" },
	{ "/Options/_Accounts",  NULL,			on_menu_accounts,0, "<Item>" },
	{ "/Options/_Contacts",  NULL,			NULL,		0, "<Item>" },


	{ "/_Help",         NULL,                       NULL,           0, "<Branch>" },
	{ "/_Help/About",   NULL,                       on_menu_about,  0, "<StockItem>", GTK_STOCK_ABOUT},
};

static gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);


static GtkWidget *
menubar_new (ModestMainWindow *self)
{
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	
	/* Make an accelerator group (shortcut keys) */
	accel_group = gtk_accel_group_new ();
	
	/* Make an ItemFactory (that makes a menubar) */
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
					     accel_group);
	
	/* This function generates the menu items. Pass the item factory,
	   the number of items in the array, the array itself, and any
	   callback data for the the menu items. */
	gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, self);
	
	///* Attach the new accelerator group to the window. */
	gtk_window_add_accel_group (GTK_WINDOW (self), accel_group);
	
	/* Finally, return the actual menu bar created by the item factory. */
	return gtk_item_factory_get_widget (item_factory, "<main>");
}




static ModestHeaderView*
header_view_new (ModestMainWindow *self)
{
	int i;
	GSList *columns = NULL;
	ModestHeaderView *header_view;
	ModestMainWindowPrivate *priv;
	ModestHeaderViewColumn cols[] = {
		MODEST_HEADER_VIEW_COLUMN_MSGTYPE,
		MODEST_HEADER_VIEW_COLUMN_ATTACH,
		MODEST_HEADER_VIEW_COLUMN_FROM,
		MODEST_HEADER_VIEW_COLUMN_SUBJECT,
		MODEST_HEADER_VIEW_COLUMN_RECEIVED_DATE
	};

	priv = MODEST_MAIN_WINDOW_GET_PRIVATE(self);
	
	for (i = 0 ; i != sizeof(cols) / sizeof(ModestHeaderViewColumn); ++i)
		columns = g_slist_append (columns, GINT_TO_POINTER(cols[i]));

	header_view = modest_widget_factory_get_header_view (priv->widget_factory);
	modest_header_view_set_columns (header_view, columns);
	g_slist_free (columns);

	return header_view;
}


static void
set_sizes (ModestMainWindow *self)
{
	ModestMainWindowPrivate *priv;

	int win_x,win_y;
	int fol_x,fol_pos;
	int msg_x,msg_pos;
	
	priv = MODEST_MAIN_WINDOW_GET_PRIVATE(self);

	/* size of the whole window */
	win_x = modest_conf_get_int_or_default (priv->conf,
						      MODEST_MAIN_WINDOW_WIDTH,
						      MODEST_MAIN_WINDOW_WIDTH_DEFAULT);
	win_y = modest_conf_get_int_or_default (priv->conf,
						      MODEST_MAIN_WINDOW_HEIGHT,
						      MODEST_MAIN_WINDOW_HEIGHT_DEFAULT);
	gtk_window_set_default_size (GTK_WINDOW(self), win_x, win_y);
	
	/* size of the folder pane */
	fol_x = modest_conf_get_int_or_default (priv->conf,
						      MODEST_FOLDER_PANED_WIDTH,
						      MODEST_FOLDER_PANED_WIDTH_DEFAULT);
	fol_pos = modest_conf_get_int_or_default (priv->conf,
							MODEST_FOLDER_PANED_DIVIDER_POS,
							MODEST_FOLDER_PANED_DIVIDER_POS_DEFAULT);
	if (1 > fol_x || fol_x > win_x || 1 > fol_pos || fol_pos > win_y) {
		g_printerr ("modest: folder paned <x,pos> out of range: <%d,%d>\n",
			    fol_x, fol_pos);
	} else {
		/* slightly off... */
		gtk_widget_set_size_request (priv->folder_paned, fol_x, win_y);
		gtk_paned_set_position (GTK_PANED(priv->folder_paned),
					fol_pos);
	}
	
	/* size of the folder pane */
	msg_x = modest_conf_get_int_or_default (priv->conf,
						      MODEST_MSG_PANED_WIDTH,
						      MODEST_MSG_PANED_WIDTH_DEFAULT);
	msg_pos = modest_conf_get_int_or_default (priv->conf,
							MODEST_MSG_PANED_DIVIDER_POS,
							MODEST_MSG_PANED_DIVIDER_POS_DEFAULT);
	if (1 > msg_x || msg_x > win_x || 1 > msg_pos || msg_pos > win_y) {
		g_printerr ("modest: msg paned <x,pos> out of range: <%d,%d>\n",
			    msg_x, msg_pos);
	} else {
		/* slightly off... */
		gtk_widget_set_size_request (priv->msg_paned,
					     fol_x, win_y);
		gtk_paned_set_position (GTK_PANED(priv->msg_paned),
					msg_pos);
	}
}



static GtkWidget*
wrapped_in_scrolled_window (GtkWidget *widget, gboolean needs_viewport)
{
	GtkWidget *win;

	win = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy
		(GTK_SCROLLED_WINDOW (win),GTK_POLICY_NEVER,
		 GTK_POLICY_AUTOMATIC);
	
	if (needs_viewport)
		gtk_scrolled_window_add_with_viewport
			(GTK_SCROLLED_WINDOW(win), widget);
	else
		gtk_container_add (GTK_CONTAINER(win),
				   widget);

	return win;
}

GtkWidget*
modest_main_window_new (ModestWidgetFactory *factory, ModestConf *conf)
{
	GObject *obj;
	ModestMainWindowPrivate *priv;
	
	GtkWidget *main_vbox;
	GtkWidget *main_paned;
	GtkWidget *status_hbox;
	GtkWidget *header_win, *folder_win; 
	
	g_return_val_if_fail (factory, NULL);
	g_return_val_if_fail (conf, NULL);

	obj  = g_object_new(MODEST_TYPE_MAIN_WINDOW, NULL);
	priv = MODEST_MAIN_WINDOW_GET_PRIVATE(obj);

	g_object_ref (factory);
	priv->widget_factory = factory;

	g_object_ref (conf);
	priv->conf = conf;

	/* widgets from factory */
	priv->folder_view = modest_widget_factory_get_folder_view (factory);
	priv->header_view = header_view_new (MODEST_MAIN_WINDOW(obj));
	priv->msg_preview = modest_widget_factory_get_msg_preview (factory);

	folder_win = wrapped_in_scrolled_window (GTK_WIDGET(priv->folder_view),
						 FALSE);
	header_win = wrapped_in_scrolled_window (GTK_WIDGET(priv->header_view),
						 FALSE);			   
	/* tool/menubar */
	priv->menubar = menubar_new (MODEST_MAIN_WINDOW(obj));
	priv->toolbar = gtk_toolbar_new ();

	/* paned */
	priv->folder_paned = gtk_vpaned_new ();
	priv->msg_paned = gtk_vpaned_new ();
	main_paned = gtk_hpaned_new ();
	gtk_paned_add1 (GTK_PANED(main_paned), priv->folder_paned);
	gtk_paned_add2 (GTK_PANED(main_paned), priv->msg_paned);
	gtk_paned_add1 (GTK_PANED(priv->folder_paned),
			gtk_label_new (_("Favorites")));
	gtk_paned_add2 (GTK_PANED(priv->folder_paned), folder_win);
	gtk_paned_add1 (GTK_PANED(priv->msg_paned), header_win);
	gtk_paned_add2 (GTK_PANED(priv->msg_paned), GTK_WIDGET(priv->msg_preview));

	gtk_widget_show (GTK_WIDGET(priv->header_view));
		
	/* status bar / progress */
	priv->status_bar   = gtk_statusbar_new ();
	priv->progress_bar = gtk_progress_bar_new ();
	status_hbox = gtk_hbox_new (TRUE, 5);
	gtk_box_pack_start (GTK_BOX(status_hbox), priv->progress_bar, FALSE, TRUE, 5);
	gtk_box_pack_start (GTK_BOX(status_hbox), priv->status_bar, FALSE, TRUE, 5);
	
	/* putting it all together... */
	main_vbox = gtk_vbox_new (FALSE, 2);
	gtk_box_pack_start (GTK_BOX(main_vbox), priv->menubar, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX(main_vbox), priv->toolbar, FALSE, TRUE, 5);
	gtk_box_pack_start (GTK_BOX(main_vbox), main_paned, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(main_vbox), status_hbox, FALSE, FALSE, 5);
	
	gtk_container_add (GTK_CONTAINER(obj), main_vbox);
	gtk_widget_show_all (main_vbox);
	
	gtk_window_set_title (GTK_WINDOW(obj), "Modest");

	set_sizes (MODEST_MAIN_WINDOW(obj));	
	return GTK_WIDGET(obj);
}
