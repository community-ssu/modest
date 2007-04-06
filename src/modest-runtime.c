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

#include <config.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <modest-runtime.h>
#include <modest-defs.h>
#include <modest-singletons.h>
#include <widgets/modest-header-view.h>
#include <widgets/modest-folder-view.h>
#include <modest-tny-platform-factory.h>
#include <modest-platform.h>
#include <modest-widget-memory.h>
#include <modest-widget-memory-priv.h>
#include <modest-local-folder-info.h>
#include <modest-account-mgr.h>
#include <modest-account-mgr-helpers.h>
#include <modest-icon-names.h>

static gboolean init_header_columns (ModestConf *conf, gboolean overwrite);
static gboolean init_local_folders  (void);
static gboolean init_default_account_maybe  (ModestAccountMgr *acc_mgr);
static void     init_i18n (void);
static void     init_stock_icons (void);
static void     init_debug_g_type (void);
static void     init_debug_logging (void);
static void     init_test_accounts (ModestAccountMgr *acc_mgr);
static void     init_default_settings (ModestConf *conf);


static ModestSingletons *_singletons = NULL;


/*
 * defaults for the column headers
 */
typedef struct {
	ModestHeaderViewColumn col;
	guint                  width;
} FolderCols;

static const FolderCols INBOX_COLUMNS_DETAILS[] = {
	{MODEST_HEADER_VIEW_COLUMN_MSGTYPE, 40},
	{MODEST_HEADER_VIEW_COLUMN_ATTACH,  40},
	{MODEST_HEADER_VIEW_COLUMN_FROM,    80},
	{MODEST_HEADER_VIEW_COLUMN_SUBJECT, 80},
	{MODEST_HEADER_VIEW_COLUMN_RECEIVED_DATE, 60},
	{MODEST_HEADER_VIEW_COLUMN_SIZE, 50}
};
static const FolderCols INBOX_COLUMNS_TWOLINES[] = {
	{MODEST_HEADER_VIEW_COLUMN_MSGTYPE, 40},
	{MODEST_HEADER_VIEW_COLUMN_ATTACH,  40},
	{MODEST_HEADER_VIEW_COLUMN_COMPACT_HEADER_IN, 200}
};

static const FolderCols OUTBOX_COLUMNS_DETAILS[] = {
	 {MODEST_HEADER_VIEW_COLUMN_MSGTYPE, 40},
	 {MODEST_HEADER_VIEW_COLUMN_ATTACH,  40},
	 {MODEST_HEADER_VIEW_COLUMN_TO,    80},
	 {MODEST_HEADER_VIEW_COLUMN_SUBJECT, 80},
	 {MODEST_HEADER_VIEW_COLUMN_SENT_DATE, 80},
	 {MODEST_HEADER_VIEW_COLUMN_SIZE, 50}
};
static const FolderCols OUTBOX_COLUMNS_TWOLINES[] = {
	 {MODEST_HEADER_VIEW_COLUMN_MSGTYPE, 40},
	 {MODEST_HEADER_VIEW_COLUMN_ATTACH,  40},
	 {MODEST_HEADER_VIEW_COLUMN_COMPACT_HEADER_OUT,200},
};

#ifdef MODEST_PLATFORM_MAEMO
static const TnyFolderType LOCAL_FOLDERS[] = {
	TNY_FOLDER_TYPE_OUTBOX,
	TNY_FOLDER_TYPE_DRAFTS,
	TNY_FOLDER_TYPE_SENT
};
#else
static const TnyFolderType LOCAL_FOLDERS[] = {
	TNY_FOLDER_TYPE_OUTBOX,
	TNY_FOLDER_TYPE_DRAFTS,
	TNY_FOLDER_TYPE_SENT,
	TNY_FOLDER_TYPE_TRASH,
	TNY_FOLDER_TYPE_ARCHIVE	
};
#endif /* MODEST_PLATFORM_MAEMO */



gboolean
modest_runtime_init (void)
{
	ModestSingletons *my_singletons;
	gboolean reset;
	
	if (_singletons) {
		g_printerr ("modest: modest_runtime_init can only be called once\n");
		return FALSE;
	}
	
	init_i18n();
	init_debug_g_type();
	init_debug_logging();

	g_thread_init(NULL);
	gdk_threads_init ();

	my_singletons = modest_singletons_new ();
	if (!my_singletons) {
		g_printerr ("modest: failed to initialize singletons\n");
		return FALSE;
	}
	
	if (!modest_platform_init()) {
		modest_runtime_uninit ();
		g_printerr ("modest: failed to run platform-specific initialization\n");
		return FALSE;
	}

	/* based on the debug settings, we decide whether to overwrite old settings */
	reset = modest_runtime_get_debug_flags () & MODEST_RUNTIME_DEBUG_FACTORY_SETTINGS;
	if (!init_header_columns(modest_singletons_get_conf (my_singletons), reset)) {
		modest_runtime_uninit ();
		g_printerr ("modest: failed to init header columns\n");
		return FALSE;
	}

	init_default_settings (modest_singletons_get_conf (my_singletons));
	
	if (!init_local_folders()) {
		modest_runtime_uninit ();
		g_printerr ("modest: failed to init local folders\n");
		return FALSE;
	}
	
	if (!init_default_account_maybe(modest_singletons_get_account_mgr (my_singletons))) {
		modest_runtime_uninit ();
		g_printerr ("modest: failed to init default account\n");
		return FALSE;
	}

	/* create the test accounts if the debug flag for that is set */
	if (modest_runtime_get_debug_flags () & MODEST_RUNTIME_DEBUG_TINY_TEST_ACCOUNTS)
		init_test_accounts (modest_singletons_get_account_mgr (my_singletons));	
	
	/* don't initialize _singletons before all the other init stuff
	 * is done; thus, using any of the singleton stuff before
	 * runtime is fully init'ed  is avoided
	 */
	_singletons = my_singletons;
	
	return TRUE;
}


gboolean
modest_runtime_init_ui (gint argc, gchar** argv)
{
	if (!gtk_init_check(&argc, &argv)) {
		g_printerr ("modest: failed to initialize graphical ui\n");
		return FALSE;
	}

	init_stock_icons ();
	return TRUE;
}


gboolean
modest_runtime_uninit (void)
{
	if (_singletons) {
		g_object_unref (G_OBJECT(_singletons));
		_singletons = NULL;
	}
	modest_runtime_verify_object_death (_singletons, "_singletons");
	
	return TRUE;
}


ModestAccountMgr*
modest_runtime_get_account_mgr   (void)
{
	g_return_val_if_fail (_singletons, NULL);
	return modest_singletons_get_account_mgr (_singletons);
}

ModestTnyAccountStore*
modest_runtime_get_account_store   (void)
{
	g_return_val_if_fail (_singletons, NULL);
	return modest_singletons_get_account_store (_singletons);

}

ModestConf*
modest_runtime_get_conf (void)
{
	g_return_val_if_fail (_singletons, NULL);
	return modest_singletons_get_conf (_singletons);
}


ModestCacheMgr*
modest_runtime_get_cache_mgr (void)
{
	g_return_val_if_fail (_singletons, NULL);
	return modest_singletons_get_cache_mgr (_singletons);
}


ModestMailOperationQueue*
modest_runtime_get_mail_operation_queue (void)
{
	g_return_val_if_fail (_singletons, NULL);
	return modest_singletons_get_mail_operation_queue (_singletons);
}



TnyDevice*
modest_runtime_get_device (void)
{
	g_return_val_if_fail (_singletons, NULL);
	return modest_singletons_get_device (_singletons);
}


TnyPlatformFactory*
modest_runtime_get_platform_factory  (void)
{
	g_return_val_if_fail (_singletons, NULL);
	return modest_singletons_get_platform_factory (_singletons);
}




ModestTnySendQueue*
modest_runtime_get_send_queue  (TnyTransportAccount *account)
{
	ModestCacheMgr *cache_mgr;
	GHashTable     *send_queue_cache;
	gpointer       orig_key, send_queue;
	
	g_return_val_if_fail (_singletons, NULL);
	g_return_val_if_fail (TNY_IS_TRANSPORT_ACCOUNT(account), NULL);

	cache_mgr = modest_singletons_get_cache_mgr (_singletons);
	send_queue_cache = modest_cache_mgr_get_cache (cache_mgr,
						       MODEST_CACHE_MGR_CACHE_TYPE_SEND_QUEUE);

	if (!g_hash_table_lookup_extended (send_queue_cache, account, &orig_key, &send_queue)) {
		send_queue = (gpointer)modest_tny_send_queue_new (TNY_CAMEL_TRANSPORT_ACCOUNT(account));
		g_hash_table_insert (send_queue_cache, account, send_queue);
	}

	return MODEST_TNY_SEND_QUEUE(send_queue);
}




/* http://primates.ximian.com/~federico/news-2006-04.html#memory-debugging-infrastructure*/
ModestRuntimeDebugFlags
modest_runtime_get_debug_flags ()
{
	static const GDebugKey debug_keys[] = {
		{ "abort-on-warning",   MODEST_RUNTIME_DEBUG_ABORT_ON_WARNING },
		{ "log-actions",        MODEST_RUNTIME_DEBUG_LOG_ACTIONS },
		{ "debug-objects",      MODEST_RUNTIME_DEBUG_DEBUG_OBJECTS },
		{ "debug-signals",      MODEST_RUNTIME_DEBUG_DEBUG_SIGNALS },
		{ "factory-settings",   MODEST_RUNTIME_DEBUG_FACTORY_SETTINGS},
		{ "tiny-test-accounts", MODEST_RUNTIME_DEBUG_TINY_TEST_ACCOUNTS}
	};
	const gchar *str;
	static ModestRuntimeDebugFlags debug_flags = -1;

	if (debug_flags != -1)
		return debug_flags;
	
	str = g_getenv (MODEST_DEBUG);
	
	if (str != NULL)
		debug_flags = g_parse_debug_string (str, debug_keys, G_N_ELEMENTS (debug_keys));
	else
		debug_flags = 0;
	
	return debug_flags;
}





/* NOTE: the exact details of this format are important, as they
 * are also used in modest-widget-memory. FIXME: make a shared function
 * for this with widget-memory
 */
static gboolean
save_header_settings (ModestConf *conf, TnyFolderType type,
		      ModestHeaderViewStyle style,  const FolderCols* cols,
		      guint col_num, gboolean overwrite)
{
	int i;
	gchar *key;
	GString *str;

	g_return_val_if_fail (cols, FALSE);

	key = _modest_widget_memory_get_keyname_with_double_type ("header-view",
								  type, style,
								  MODEST_WIDGET_MEMORY_PARAM_COLUMN_WIDTH);
	/* if we're not in overwrite mode, only write stuff it
	 * there was nothing before */
	if (!overwrite &&  modest_conf_key_exists(conf, key, NULL)) {
		g_free (key);
		return TRUE;
	}

	/* the format is necessarily the same as the one in modest-widget-memory */
	str = g_string_new (NULL);
	for (i = 0; i != col_num; ++i) 
		g_string_append_printf (str, "%d:%d ",
					cols[i].col, cols[i].width); 

	modest_conf_set_string (conf, key, str->str, NULL);
	g_free (key);
	g_string_free (str, TRUE);
	
	return TRUE;
}

/**
 * modest_init_header_columns:
 * @overwrite: write the setting, even if it already exists
 * 
 * will set defaults for the columns to show for folder,
 * if there are no such settings yet (in ModestWidgetMemory)
 * 
 * Returns: TRUE if succeeded, FALSE in case of error
 */
static gboolean
init_header_columns (ModestConf *conf, gboolean overwrite)
{
	int folder_type;
	
	for (folder_type = TNY_FOLDER_TYPE_UNKNOWN;
	     folder_type <= TNY_FOLDER_TYPE_CALENDAR; ++folder_type) {		
		
		switch (folder_type) {
		case TNY_FOLDER_TYPE_OUTBOX:
		case TNY_FOLDER_TYPE_SENT:
		case TNY_FOLDER_TYPE_DRAFTS:
		save_header_settings (conf, folder_type,
				      MODEST_HEADER_VIEW_STYLE_DETAILS,
				      OUTBOX_COLUMNS_DETAILS,
				      G_N_ELEMENTS(OUTBOX_COLUMNS_DETAILS),
				      overwrite);
		save_header_settings (conf, folder_type,
				      MODEST_HEADER_VIEW_STYLE_TWOLINES,
				      OUTBOX_COLUMNS_TWOLINES,
				      G_N_ELEMENTS(OUTBOX_COLUMNS_TWOLINES),
				      overwrite);
		break;

		default:
		save_header_settings (conf, folder_type,
				      MODEST_HEADER_VIEW_STYLE_DETAILS,
				      INBOX_COLUMNS_DETAILS,
				      G_N_ELEMENTS(INBOX_COLUMNS_DETAILS),
				      overwrite);
		save_header_settings (conf, folder_type,
				      MODEST_HEADER_VIEW_STYLE_TWOLINES,
				      INBOX_COLUMNS_TWOLINES,
				      G_N_ELEMENTS(INBOX_COLUMNS_TWOLINES),
				      overwrite);
		};
	}
	return TRUE;
}

/**
 * init_local_folders:
 * 
 * create the Local Folders folder under cache, if they
 * do not exist yet.
 * 
 * Returns: TRUE if the folder were already there, or
 * they were created, FALSE otherwise
 */
static gboolean
init_local_folders  (void)
{
	int i;
	gchar *maildir_path;
	static const gchar* maildirs[] = {
		"cur", "new", "tmp"
	};
	
	maildir_path = modest_local_folder_info_get_maildir_path ();

	for (i = 0; i != G_N_ELEMENTS(LOCAL_FOLDERS); ++i) {
		int j;
		for (j = 0; j != G_N_ELEMENTS(maildirs); ++j) {
			gchar *dir;
			dir = g_build_filename (maildir_path,
						modest_local_folder_info_get_type_name(LOCAL_FOLDERS[i]),
						maildirs[j],
						NULL);
			if (g_mkdir_with_parents (dir, 0755) < 0) {
				g_printerr ("modest: failed to create %s\n", dir);
				g_free (dir);
				g_free (maildir_path);
				return FALSE;
			}
			g_free(dir);
		}
	}
	
	g_free (maildir_path);
	return TRUE;
}



static void
free_element (gpointer data, gpointer user_data)
{
	g_free (data);
}



/**
 * init_default_account_maybe:
 *
 * if there are accounts defined, but there is no default account,
 * it will be defined.
 * 
 * Returns: TRUE if there was a default account already,
 *  or one has been created or there are no accounts yet,
 *  returns FALSE in case of error
 */
static gboolean
init_default_account_maybe  (ModestAccountMgr *acc_mgr)
{
	GSList *all_accounts = NULL;
	gchar *default_account;
	gboolean retval = TRUE;
	
	all_accounts = modest_account_mgr_account_names (acc_mgr);
	if (all_accounts) { /* if there are any accounts, there should be a default one */
		default_account = 
			modest_account_mgr_get_default_account (acc_mgr);
		if (!default_account) {
			gchar *first_account;
			g_printerr ("modest: no default account defined\n");
			first_account = (gchar*)all_accounts->data;
			if ((retval = modest_account_mgr_set_default_account (acc_mgr, first_account)))
				g_printerr ("modest: set '%s' as the default account\n",
					    first_account);
			else
				g_printerr ("modest: failed to set '%s' as the default account\n",
					    first_account);
			g_free (default_account);
		}
		g_slist_foreach (all_accounts, free_element, NULL);
		g_slist_free    (all_accounts);
	}
	return retval;
}



static void
init_debug_g_type (void)
{
	GTypeDebugFlags gflags;
	ModestRuntimeDebugFlags mflags;
	
	gflags = 0;
	mflags = modest_runtime_get_debug_flags ();

	if (mflags & MODEST_RUNTIME_DEBUG_DEBUG_OBJECTS)
		gflags |= G_TYPE_DEBUG_OBJECTS;
	if (mflags & MODEST_RUNTIME_DEBUG_DEBUG_SIGNALS)
		gflags |= G_TYPE_DEBUG_SIGNALS;

	g_type_init_with_debug_flags (gflags);
}

static void
init_debug_logging (void)
{
	ModestRuntimeDebugFlags mflags;
	mflags = modest_runtime_get_debug_flags ();
	
	if (mflags & MODEST_RUNTIME_DEBUG_ABORT_ON_WARNING)
		g_log_set_always_fatal (G_LOG_LEVEL_ERROR |
					G_LOG_LEVEL_CRITICAL |
					G_LOG_LEVEL_WARNING);
}


static void
init_i18n (void)
{
	/* Setup gettext, to use our .po files: */
	/* GETTEXT_PACKAGE and MODEST_LOCALE_DIR are defined in config.h */
	bindtextdomain (GETTEXT_PACKAGE, MODEST_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
}


/* 
 *  This function registers our custom toolbar icons, so they can be
 *  themed. The idea of this function was taken from the gtk-demo
 */
static void
init_stock_icons (void)
{
	static gboolean registered = FALSE;
  
	if (!registered) {
		GtkIconTheme *current_theme;
		GdkPixbuf *pixbuf;
		GtkIconFactory *factory;
		gint i;

		static GtkStockItem items[] = {
#ifdef MODEST_PLATFORM_MAEMO
			{ MODEST_STOCK_SPLIT_VIEW, "split view", 0, 0, NULL },
			{ MODEST_STOCK_SORT, "sort mail", 0, 0, NULL },
			{ MODEST_STOCK_REFRESH, "refresh mail", 0, 0, NULL },
#endif /*MODEST_PLATFORM_MAEMO*/
			{ MODEST_STOCK_MAIL_SEND, "send mail", 0, 0, NULL },
			{ MODEST_STOCK_NEW_MAIL, "new mail", 0, 0, NULL },
/*  			{ MODEST_STOCK_SEND_RECEIVE, "send receive", 0, 0, NULL },  */
			{ MODEST_STOCK_REPLY, "reply", 0, 0, NULL },
			{ MODEST_STOCK_REPLY_ALL, "reply all", 0, 0, NULL },
			{ MODEST_STOCK_FORWARD, "forward", 0, 0, NULL },
 			{ MODEST_STOCK_DELETE, "delete", 0, 0, NULL }, 
/* 			{ MODEST_STOCK_NEXT, "next", 0, 0, NULL }, */
/* 			{ MODEST_STOCK_PREV, "prev", 0, 0, NULL }, */
/* 			{ MODEST_STOCK_STOP, "stop", 0, 0, NULL } */
		};
      
		static gchar *items_names [] = {
#ifdef MODEST_PLATFORM_MAEMO
			MODEST_TOOLBAR_ICON_SPLIT_VIEW,
			MODEST_TOOLBAR_ICON_SORT,
			MODEST_TOOLBAR_ICON_REFRESH,
#endif /*MODEST_PLATFORM_MAEMO*/
			MODEST_TOOLBAR_ICON_MAIL_SEND,
			MODEST_TOOLBAR_ICON_NEW_MAIL,
/*  			MODEST_TOOLBAR_ICON_SEND_RECEIVE,  */
			MODEST_TOOLBAR_ICON_REPLY,	
			MODEST_TOOLBAR_ICON_REPLY_ALL,
			MODEST_TOOLBAR_ICON_FORWARD,
 			MODEST_TOOLBAR_ICON_DELETE, 
/* 			MODEST_TOOLBAR_ICON_NEXT, */
/* 			MODEST_TOOLBAR_ICON_PREV, */
/* 			MODEST_TOOLBAR_ICON_STOP */
			MODEST_TOOLBAR_ICON_FORMAT_BULLETS,
		};

		registered = TRUE;

		/* Register our stock items */
		gtk_stock_add (items, G_N_ELEMENTS (items));
      
		/* Add our custom icon factory to the list of defaults */
		factory = gtk_icon_factory_new ();
		gtk_icon_factory_add_default (factory);

		current_theme = gtk_icon_theme_get_default ();

		/* Register icons to accompany stock items */
		for (i = 0; i < G_N_ELEMENTS (items); i++) {

#ifdef MODEST_PLATFORM_MAEMO  /* MODES_PLATFORM_ID: 1 ==> gnome, 2==> maemo */ 
			pixbuf = gtk_icon_theme_load_icon (current_theme,
							   items_names[i],
							   26,
							   GTK_ICON_LOOKUP_NO_SVG,
							   NULL);
#else
			pixbuf = gdk_pixbuf_new_from_file (items_names[i], NULL);
#endif

			if (pixbuf != NULL) {
				GtkIconSet *icon_set;
				GdkPixbuf *transparent;

				transparent = gdk_pixbuf_add_alpha (pixbuf, TRUE, 0xff, 0xff, 0xff);
				icon_set = gtk_icon_set_new_from_pixbuf (transparent);
				gtk_icon_factory_add (factory, items[i].stock_id, icon_set);
				gtk_icon_set_unref (icon_set);
				g_object_unref (pixbuf);
				g_object_unref (transparent);
			}
			else
				g_warning ("failed to load %s icon", items_names[i]);
		}
		/* Drop our reference to the factory, GTK will hold a reference. */
		g_object_unref (factory);
	}
}


static void
init_test_accounts (ModestAccountMgr *acc_mgr)
{
	const gchar *imap_test = "TnyImapTest";
	const gchar *pop_test  = "TnyPopTest";
	const gchar *smtp_test = "TnySmtpTest";

	if (!modest_account_mgr_account_exists (acc_mgr, imap_test, FALSE)) 
		modest_account_mgr_add_server_account (acc_mgr,
						       imap_test,
						       "imap2.tinymail.org",
						       "tnytest", "tnytest",
						       MODEST_PROTOCOL_STORE_IMAP,
						       MODEST_PROTOCOL_SECURITY_NONE,
						       MODEST_PROTOCOL_AUTH_NONE);
	if (!modest_account_mgr_account_exists (acc_mgr, pop_test, FALSE)) 
		modest_account_mgr_add_server_account (acc_mgr,
						       pop_test,
						       "imap2.tinymail.org",
						       "tnytest", "tnytest",
						       MODEST_PROTOCOL_STORE_POP,
						       MODEST_PROTOCOL_SECURITY_NONE,
						       MODEST_PROTOCOL_AUTH_NONE);
	if (!modest_account_mgr_account_exists (acc_mgr, smtp_test, FALSE)) 
		modest_account_mgr_add_server_account (acc_mgr,
						       smtp_test,
						       "127.0.0.1",
						       NULL,NULL,
						       MODEST_PROTOCOL_TRANSPORT_SMTP,
						       MODEST_PROTOCOL_SECURITY_NONE,
						       MODEST_PROTOCOL_AUTH_NONE);
	
	modest_account_mgr_add_account (acc_mgr, imap_test, imap_test, smtp_test);
	modest_account_mgr_add_account (acc_mgr,  pop_test, pop_test,  smtp_test);
}


static void
init_default_settings (ModestConf *conf)
{
	if (!modest_conf_key_exists (conf, MODEST_CONF_SHOW_TOOLBAR, NULL))
		modest_conf_set_bool (conf, MODEST_CONF_SHOW_TOOLBAR, TRUE, NULL);
	
	if (!modest_conf_key_exists (conf, MODEST_CONF_SHOW_CC, NULL))
		modest_conf_set_bool (conf, MODEST_CONF_SHOW_CC, TRUE, NULL);

	if (!modest_conf_key_exists (conf, MODEST_CONF_SHOW_BCC, NULL))
		modest_conf_set_bool (conf, MODEST_CONF_SHOW_BCC, FALSE, NULL);

	if (!modest_conf_key_exists (conf, MODEST_CONF_CONNECT_AT_STARTUP, NULL))
		modest_conf_set_bool (conf, MODEST_CONF_CONNECT_AT_STARTUP, TRUE, NULL);

}
