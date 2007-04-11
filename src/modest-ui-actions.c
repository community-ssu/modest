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
#include <string.h>
#include <modest-runtime.h>
#include <modest-tny-msg.h>
#include <modest-tny-account.h>
#include <modest-address-book.h>

#include "modest-ui-actions.h"

#include "modest-tny-platform-factory.h"
#include "modest-platform.h"

#include <widgets/modest-main-window.h>
#include <widgets/modest-msg-view-window.h>
#include <widgets/modest-account-view-window.h>
#include <widgets/modest-msg-view-details-dialog.h>

#include "modest-account-mgr-helpers.h"
#include "modest-mail-operation.h"

#ifdef MODEST_HAVE_EASYSETUP
#include "easysetup/modest-easysetup-wizard.h"
#endif /*MODEST_HAVE_EASYSETUP*/

#include <modest-widget-memory.h>
#include <tny-error.h>
#include <tny-simple-list.h>
#include <tny-msg-view.h>
#include <tny-device.h>


typedef struct _GetMsgAsyncHelper {
	ModestWindow *window;
	TnyIterator *iter;
	GFunc func;
	gpointer user_data;
} GetMsgAsyncHelper;

typedef enum _ReplyForwardAction {
	ACTION_REPLY,
	ACTION_REPLY_TO_ALL,
	ACTION_FORWARD
} ReplyForwardAction;

typedef struct _ReplyForwardHelper {
guint reply_forward_type;
	ReplyForwardAction action;
	gchar *account_name;
} ReplyForwardHelper;


static void     reply_forward_func     (gpointer data, gpointer user_data);
static void     read_msg_func          (gpointer data, gpointer user_data);
static void     get_msg_cb             (TnyFolder *folder, TnyMsg *msg,	GError **err, 
					gpointer user_data);
static void     reply_forward          (ReplyForwardAction action, ModestWindow *win);
static void     modest_ui_actions_message_details_cb (gpointer msg_data, 
						      gpointer helper_data);
static gchar*   ask_for_folder_name    (GtkWindow *parent_window, const gchar *title);


void   
modest_ui_actions_on_about (GtkAction *action, ModestWindow *win)
{
	GtkWidget *about;
	const gchar *authors[] = {
		"Dirk-Jan C. Binnema <dirk-jan.binnema@nokia.com>",
		NULL
	};
	about = gtk_about_dialog_new ();
	gtk_about_dialog_set_name (GTK_ABOUT_DIALOG(about), PACKAGE_NAME);
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about),PACKAGE_VERSION);
	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG(about),
					_("Copyright (c) 2006, Nokia Corporation\n"
					  "All rights reserved."));
	gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG(about),
				       _("a modest e-mail client\n\n"
					 "design and implementation: Dirk-Jan C. Binnema\n"
					 "contributions from the fine people at KernelConcepts and Igalia\n"
					 "uses the tinymail email framework written by Philip van Hoof"));
	gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG(about), authors);
	gtk_about_dialog_set_website (GTK_ABOUT_DIALOG(about), "http://modest.garage.maemo.org");
	
	gtk_dialog_run (GTK_DIALOG (about));
	gtk_widget_destroy(about);
}


static TnyList *
get_selected_headers (ModestWindow *win)
{
	if (MODEST_IS_MAIN_WINDOW(win)) {
		GtkWidget *header_view;		
		
		header_view = modest_main_window_get_child_widget (MODEST_MAIN_WINDOW(win),
								   MODEST_WIDGET_TYPE_HEADER_VIEW);
		return modest_header_view_get_selected_headers (MODEST_HEADER_VIEW(header_view));
		
	} else if (MODEST_IS_MSG_VIEW_WINDOW (win)) {
		/* for MsgViewWindows, we simply return a list with one element */
		TnyMsg *msg;
		TnyHeader *header;
		TnyList *list = NULL;
		
		msg  = modest_msg_view_window_get_message (MODEST_MSG_VIEW_WINDOW(win));
		if (msg) {
			header = tny_msg_get_header (msg);
			list = tny_simple_list_new ();
			tny_list_prepend (list, G_OBJECT(header));
			g_object_unref (G_OBJECT(header));
		}
		return list;

	} else
		return NULL;
}

void
modest_ui_actions_on_delete (GtkAction *action, ModestWindow *win)
{
	TnyList *header_list;
	TnyIterator *iter;

	g_return_if_fail (MODEST_IS_WINDOW(win));
		
	header_list = get_selected_headers (win);
	
	if (header_list) {
		iter = tny_list_create_iterator (header_list);
		do {
			TnyHeader *header;
			ModestMailOperation *mail_op;

			header = TNY_HEADER (tny_iterator_get_current (iter));
			/* TODO: thick grain mail operation involving
			   a list of objects. Composite pattern ??? */
			/* TODO: add confirmation dialog */
			mail_op = modest_mail_operation_new ();
			modest_mail_operation_queue_add (modest_runtime_get_mail_operation_queue (),
							 mail_op);

			/* Always delete. TODO: Move to trash still not supported */
			modest_mail_operation_remove_msg (mail_op, header, FALSE);

			/* Frees */
			g_object_unref (G_OBJECT (mail_op));
			g_object_unref (G_OBJECT (header));

			tny_iterator_next (iter);

		} while (!tny_iterator_is_done (iter));

		/* Free iter */
		g_object_unref (G_OBJECT (iter));
	}

	if (MODEST_IS_MSG_VIEW_WINDOW (win)) {
		gtk_widget_destroy (GTK_WIDGET(win));
	}
}


void
modest_ui_actions_on_quit (GtkAction *action, ModestWindow *win)
{
	/* FIXME: save size of main window */
/* 	save_sizes (main_window); */
/* 	gtk_widget_destroy (GTK_WIDGET (win)); */
	gtk_main_quit ();
}

void
modest_ui_actions_on_close_window (GtkAction *action, ModestWindow *win)
{
	if (MODEST_IS_MSG_VIEW_WINDOW (win)) {
		gtk_widget_destroy (GTK_WIDGET (win));
	} else if (MODEST_IS_WINDOW (win)) {
		gtk_widget_destroy (GTK_WIDGET (win));
	} else {
		g_return_if_reached ();
	}
}

void
modest_ui_actions_on_add_to_contacts (GtkAction *action, ModestWindow *win)
{
	GtkClipboard *clipboard = NULL;
	gchar *selection = NULL;

	clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
	selection = gtk_clipboard_wait_for_text (clipboard);

	modest_address_book_add_address (selection);
	g_free (selection);
}

void
modest_ui_actions_on_accounts (GtkAction *action, ModestWindow *win)
{
	
	/* This is currently only implemented for Maemo,
	 * because it requires a providers preset file which is not publically available.
	 */
#ifdef MODEST_PLATFORM_MAEMO /* Defined in config.h */
	GSList *account_names = modest_account_mgr_account_names (modest_runtime_get_account_mgr());
	gboolean accounts_exist = account_names != NULL;
	g_slist_free (account_names);
	
/* To test, while modest_account_mgr_account_names() is broken: accounts_exist = TRUE; */
	if (!accounts_exist) {
		/* If there are no accounts yet, just show the easy-setup wizard, as per the UI spec: */
		ModestEasysetupWizardDialog *wizard = modest_easysetup_wizard_dialog_new ();
		gtk_window_set_transient_for (GTK_WINDOW (wizard), GTK_WINDOW (win));
		gtk_dialog_run (GTK_DIALOG (wizard));
		gtk_widget_destroy (GTK_WIDGET (wizard));
	} else 	{
		/* Show the list of accounts: */
		GtkDialog *account_win = GTK_DIALOG(modest_account_view_window_new ());
		gtk_window_set_transient_for (GTK_WINDOW (account_win), GTK_WINDOW(win));
		gtk_dialog_run (account_win);
		gtk_widget_destroy (GTK_WIDGET(account_win));
	}
#else
	GtkWidget *dialog, *label;
	
	/* Create the widgets */
	
	dialog = gtk_dialog_new_with_buttons ("Message",
					      GTK_WINDOW(win),
					      GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_STOCK_OK,
					      GTK_RESPONSE_NONE,
					      NULL);
	label = gtk_label_new ("Hello World!");
	
	/* Ensure that the dialog box is destroyed when the user responds. */
	
	g_signal_connect_swapped (dialog, "response", 
				  G_CALLBACK (gtk_widget_destroy),
				  dialog);
	
	/* Add the label, and show everything we've added to the dialog. */
	
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
			   label);
	gtk_widget_show_all (dialog);
#endif /* MODEST_PLATFORM_MAEMO */
}

void
modest_ui_actions_on_new_msg (GtkAction *action, ModestWindow *win)
{
	ModestWindow *msg_win;
	TnyMsg *msg = NULL;
	TnyFolder *folder = NULL;
	gchar *account_name = NULL;
	gchar *from_str = NULL;
	GError *err = NULL;
	TnyAccount *account;
	ModestWindowMgr *mgr;
	
	account_name = g_strdup(modest_window_get_active_account (win));
	if (!account_name)
		account_name = modest_account_mgr_get_default_account (modest_runtime_get_account_mgr());
	
	account = modest_tny_account_store_get_tny_account_by_account (modest_runtime_get_account_store(),
								       account_name,
								       TNY_ACCOUNT_TYPE_STORE);
	if (!account) {
		g_printerr ("modest: failed to get tnyaccount for '%s'\n", account_name);
		goto cleanup;
	}

	from_str = modest_account_mgr_get_from_string (modest_runtime_get_account_mgr(), account_name);

	msg    = modest_tny_msg_new ("", from_str, "", "", "", "", NULL);
	if (!msg) {
		g_printerr ("modest: failed to create new msg\n");
		goto cleanup;
	}
	
	folder = modest_tny_account_get_special_folder (account, TNY_FOLDER_TYPE_DRAFTS);
	if (!folder) {
		g_printerr ("modest: failed to find Drafts folder\n");
		goto cleanup;
	}
	
	tny_folder_add_msg (folder, msg, &err);
	if (err) {
		g_printerr ("modest: error adding msg to Drafts folder: %s",
			    err->message);
		g_error_free (err);
		goto cleanup;
	}

	/* Create and register edit window */
	msg_win = modest_msg_edit_window_new (msg, account_name);
	mgr = modest_runtime_get_window_mgr ();
	modest_window_mgr_register_window (mgr, msg_win);

	if (win)
		gtk_window_set_transient_for (GTK_WINDOW (msg_win),
					      GTK_WINDOW (win));	
	gtk_widget_show_all (GTK_WIDGET (msg_win));

cleanup:
	g_free (account_name);
	g_free (from_str);
	if (account)
		g_object_unref (G_OBJECT(account));
	if (msg)
		g_object_unref (G_OBJECT(msg));
	if (folder)
		g_object_unref (G_OBJECT(folder));
}


void
modest_ui_actions_on_open (GtkAction *action, ModestWindow *win)
{
	modest_runtime_not_implemented (GTK_WINDOW(win)); /* FIXME */
}



static void
reply_forward_func (gpointer data, gpointer user_data)
{
	TnyMsg *msg, *new_msg;
	GetMsgAsyncHelper *helper;
	ReplyForwardHelper *rf_helper;
	ModestWindow *msg_win;
	ModestEditType edit_type;
	gchar *from;
	GError *err = NULL;
	TnyFolder *folder = NULL;
	TnyAccount *account = NULL;
	ModestWindowMgr *mgr;
	
	msg = TNY_MSG (data);
	helper = (GetMsgAsyncHelper *) user_data;
	rf_helper = (ReplyForwardHelper *) helper->user_data;

	from = modest_account_mgr_get_from_string (modest_runtime_get_account_mgr(),
						   rf_helper->account_name);
	/* Create reply mail */
	switch (rf_helper->action) {
	case ACTION_REPLY:
		new_msg = 
			modest_tny_msg_create_reply_msg (msg,  from, 
							 rf_helper->reply_forward_type,
							 MODEST_TNY_MSG_REPLY_MODE_SENDER);
		break;
	case ACTION_REPLY_TO_ALL:
		new_msg = 
			modest_tny_msg_create_reply_msg (msg, from, rf_helper->reply_forward_type,
							 MODEST_TNY_MSG_REPLY_MODE_ALL);
		edit_type = MODEST_EDIT_TYPE_REPLY;
		break;
	case ACTION_FORWARD:
		new_msg = 
			modest_tny_msg_create_forward_msg (msg, from, rf_helper->reply_forward_type);
		edit_type = MODEST_EDIT_TYPE_FORWARD;
		break;
	default:
		g_return_if_reached ();
		return;
	}

	if (!new_msg) {
		g_printerr ("modest: failed to create message\n");
		goto cleanup;
	}

	account = modest_tny_account_store_get_tny_account_by_account (modest_runtime_get_account_store(),
								       rf_helper->account_name,
								       TNY_ACCOUNT_TYPE_STORE);
	if (!account) {
		g_printerr ("modest: failed to get tnyaccount for '%s'\n", rf_helper->account_name);
		goto cleanup;
	}

	folder = modest_tny_account_get_special_folder (account, TNY_FOLDER_TYPE_DRAFTS);
	if (!folder) {
		g_printerr ("modest: failed to find Drafts folder\n");
		goto cleanup;
	}
	
	tny_folder_add_msg (folder, msg, &err);
	if (err) {
		g_printerr ("modest: error adding msg to Drafts folder: %s",
			    err->message);
		g_error_free (err);
		goto cleanup;
	}	

	/* Create and register the windows */			
	msg_win = modest_msg_edit_window_new (new_msg, rf_helper->account_name);
	mgr = modest_runtime_get_window_mgr ();
	modest_window_mgr_register_window (mgr, msg_win);

	/* Show edit window */
	gtk_widget_show_all (GTK_WIDGET (msg_win));

cleanup:
	if (new_msg)
		g_object_unref (G_OBJECT (new_msg));
	if (folder)
		g_object_unref (G_OBJECT (folder));
	if (account)
		g_object_unref (G_OBJECT (account));
	
	g_free (rf_helper->account_name);
	g_slice_free (ReplyForwardHelper, rf_helper);
}
/*
 * Common code for the reply and forward actions
 */
static void
reply_forward (ReplyForwardAction action, ModestWindow *win)
{
	TnyList *header_list;
	guint reply_forward_type;
	TnyHeader *header;
	TnyFolder *folder;
	GetMsgAsyncHelper *helper;
	ReplyForwardHelper *rf_helper;
	
	g_return_if_fail (MODEST_IS_WINDOW(win));

	header_list = get_selected_headers (win);
	if (!header_list)
		return;
	
	reply_forward_type = modest_conf_get_int (modest_runtime_get_conf (),
						  (action == ACTION_FORWARD) ? MODEST_CONF_FORWARD_TYPE : MODEST_CONF_REPLY_TYPE,
						  NULL);
	/* We assume that we can only select messages of the
	   same folder and that we reply all of them from the
	   same account. In fact the interface currently only
	   allows single selection */
	
	/* Fill helpers */
	rf_helper = g_slice_new0 (ReplyForwardHelper);
	rf_helper->reply_forward_type = reply_forward_type;
	rf_helper->action = action;

	rf_helper->account_name = g_strdup (modest_window_get_active_account (win));
	if (!rf_helper->account_name)
		rf_helper->account_name =
			modest_account_mgr_get_default_account (modest_runtime_get_account_mgr());

	helper = g_slice_new0 (GetMsgAsyncHelper);
	helper->window = win;
	helper->func = reply_forward_func;
	helper->iter = tny_list_create_iterator (header_list);
	helper->user_data = rf_helper;

	if (MODEST_IS_MSG_VIEW_WINDOW(win)) {
		TnyMsg *msg;
		msg = modest_msg_view_window_get_message(MODEST_MSG_VIEW_WINDOW(win));
		if (!msg) {
			g_printerr ("modest: no message found\n");
			return;
		} else
			reply_forward_func (msg, helper);
	} else {
		header = TNY_HEADER (tny_iterator_get_current (helper->iter));
		folder = tny_header_get_folder (header);
		if (folder) {
			/* The callback will call it per each header */
			tny_folder_get_msg_async (folder, header, get_msg_cb, helper);
			g_object_unref (G_OBJECT (folder));
		} else 
			g_printerr ("modest: no folder for header\n");
		
		/* Clean */
		g_object_unref (G_OBJECT (header));
	}
}


void
modest_ui_actions_on_reply (GtkAction *action, ModestWindow *win)
{
	g_return_if_fail (MODEST_IS_WINDOW(win));

	reply_forward (ACTION_REPLY, win);
}

void
modest_ui_actions_on_forward (GtkAction *action, ModestWindow *win)
{
	g_return_if_fail (MODEST_IS_WINDOW(win));

	reply_forward (ACTION_FORWARD, win);
}

void
modest_ui_actions_on_reply_all (GtkAction *action, ModestWindow *win)
{
	g_return_if_fail (MODEST_IS_WINDOW(win));

	reply_forward (ACTION_REPLY_TO_ALL, win);
}

void 
modest_ui_actions_on_next (GtkAction *action, 
			   ModestWindow *window)
{
	if (MODEST_IS_MAIN_WINDOW (window)) {
		GtkWidget *header_view;

		header_view = modest_main_window_get_child_widget (MODEST_MAIN_WINDOW(window),
								   MODEST_WIDGET_TYPE_HEADER_VIEW);
		if (!header_view)
			return;
	
		modest_header_view_select_next (MODEST_HEADER_VIEW(header_view)); 
	} else if (MODEST_IS_MSG_VIEW_WINDOW (window)) {
		modest_msg_view_window_select_next_message (MODEST_MSG_VIEW_WINDOW (window));
	} else {
		g_return_if_reached ();
	}
}

void 
modest_ui_actions_on_prev (GtkAction *action, 
			   ModestWindow *window)
{
	g_return_if_fail (MODEST_IS_WINDOW(window));

	if (MODEST_IS_MAIN_WINDOW (window)) {
		GtkWidget *header_view;
		header_view = modest_main_window_get_child_widget (MODEST_MAIN_WINDOW(window),
								   MODEST_WIDGET_TYPE_HEADER_VIEW);
		if (!header_view)
			return;
		
		modest_header_view_select_prev (MODEST_HEADER_VIEW(header_view)); 
	} else if (MODEST_IS_MSG_VIEW_WINDOW (window)) {
		modest_msg_view_window_select_previous_message (MODEST_MSG_VIEW_WINDOW (window));
	} else {
		g_return_if_reached ();
	}
}


static gboolean
action_send (const gchar* account_name)
{
	TnyAccount *tny_account;
	ModestTnySendQueue *send_queue;

	g_return_val_if_fail (account_name, FALSE);

	tny_account = 
		modest_tny_account_store_get_tny_account_by_account (modest_runtime_get_account_store(),
								     account_name,
								     TNY_ACCOUNT_TYPE_TRANSPORT);
	if (!tny_account) {
		g_printerr ("modest: cannot get tny transport account for %s\n", account_name);
		return FALSE;
	}
	send_queue = modest_tny_send_queue_new (TNY_CAMEL_TRANSPORT_ACCOUNT(tny_account));
	if (!send_queue) {
		g_object_unref (G_OBJECT(tny_account));
		g_printerr ("modest: cannot get send queue for %s\n", account_name);
		return FALSE;
	}
	
	//modest_tny_send_queue_flush (send_queue);

	g_object_unref (G_OBJECT(send_queue));
	g_object_unref (G_OBJECT(tny_account));

	return TRUE;
}


static gboolean
action_receive (const gchar* account_name)
{
	TnyAccount *tny_account;
	ModestMailOperation *mail_op;

	g_return_val_if_fail (account_name, FALSE);

	tny_account = 
		modest_tny_account_store_get_tny_account_by_account (modest_runtime_get_account_store(),
								     account_name,
								     TNY_ACCOUNT_TYPE_STORE);
	if (!tny_account) {
		g_printerr ("modest: cannot get tny store account for %s\n", account_name);
		return FALSE;
	}

	/* Create the mail operation */
	mail_op = modest_mail_operation_new ();
	modest_mail_operation_queue_add (modest_runtime_get_mail_operation_queue (), mail_op);
	modest_mail_operation_update_account (mail_op, TNY_STORE_ACCOUNT(tny_account));
	g_object_unref (G_OBJECT(tny_account));
	g_object_unref (G_OBJECT (mail_op));
		
	return TRUE;
}



void
modest_ui_actions_on_send_receive (GtkAction *action,  ModestWindow *win)
{
	gchar *account_name;
	
	account_name =
		g_strdup(modest_window_get_active_account(MODEST_WINDOW(win)));
	if (!account_name)
		account_name  = modest_account_mgr_get_default_account (modest_runtime_get_account_mgr());
	if (!account_name) {
		g_printerr ("modest: cannot get account\n");
		return;
	}

	if (!action_send(account_name))
		g_printerr ("modest: failed to send\n");
	if (!action_receive(account_name))
		g_printerr ("modest: failed to receive\n");
}



void
modest_ui_actions_toggle_view (GtkAction *action, ModestMainWindow *main_window)
{
	ModestConf *conf;
	GtkWidget *header_view;
	
	g_return_if_fail (MODEST_IS_MAIN_WINDOW(main_window));

	header_view = modest_main_window_get_child_widget (main_window,
							   MODEST_WIDGET_TYPE_HEADER_VIEW);
	if (!header_view)
		return;

	conf = modest_runtime_get_conf ();
	
	/* what is saved/restored is depending on the style; thus; we save with
	 * old style, then update the style, and restore for this new style
	 */
	modest_widget_memory_save (conf, G_OBJECT(header_view), "header-view");
	
	if (modest_header_view_get_style
	    (MODEST_HEADER_VIEW(header_view)) == MODEST_HEADER_VIEW_STYLE_DETAILS)
		modest_header_view_set_style (MODEST_HEADER_VIEW(header_view),
					      MODEST_HEADER_VIEW_STYLE_TWOLINES);
	else
		modest_header_view_set_style (MODEST_HEADER_VIEW(header_view),
					      MODEST_HEADER_VIEW_STYLE_DETAILS);

	modest_widget_memory_restore (conf, G_OBJECT(header_view),
				      "header-view");
}



/*
 * Marks a message as read and passes it to the msg preview widget
 */
static void
read_msg_func (gpointer data, gpointer user_data)
{
	TnyMsg *msg;
	TnyHeader *header;
	GetMsgAsyncHelper *helper;
	TnyHeaderFlags header_flags;
	GtkWidget *msg_preview;
	
	msg = TNY_MSG (data);
	helper = (GetMsgAsyncHelper *) user_data;

	msg_preview = modest_main_window_get_child_widget (MODEST_MAIN_WINDOW (helper->window),
							   MODEST_WIDGET_TYPE_MSG_PREVIEW);
	if (!msg_preview)
		return;
	
	header = TNY_HEADER (tny_iterator_get_current (helper->iter));
	header_flags = tny_header_get_flags (header);
	tny_header_set_flags (header, header_flags | TNY_HEADER_FLAG_SEEN);
	g_object_unref (G_OBJECT (header));

	/* Set message on msg view */
	modest_msg_view_set_message (MODEST_MSG_VIEW(msg_preview), msg);
}

/*
 * This function is a generic handler for the tny_folder_get_msg_async
 * call. It expects as user_data a #GetMsgAsyncHelper. This helper
 * contains a user provided function that is called inside this
 * method. This will allow us to use this callback in many different
 * places. This callback performs the common actions for the
 * get_msg_async call, more specific actions will be done by the user
 * function
 */
static void
get_msg_cb (TnyFolder *folder, TnyMsg *msg, GError **err, gpointer user_data)
{
	GetMsgAsyncHelper *helper;

	helper = (GetMsgAsyncHelper *) user_data;

	if ((*err && ((*err)->code == TNY_FOLDER_ERROR_GET_MSG)) || !msg) {
		modest_ui_actions_on_item_not_found (NULL,
						     MODEST_ITEM_TYPE_MESSAGE,
						     helper->window);
		return;
	}

	/* Call user function */
	helper->func (msg, user_data);

	/* Process next element (if exists) */
	tny_iterator_next (helper->iter);
	if (tny_iterator_is_done (helper->iter)) {
		TnyList *headers;
		headers = tny_iterator_get_list (helper->iter);
		/* Free resources */
		g_object_unref (G_OBJECT (headers));
		g_object_unref (G_OBJECT (helper->iter));
		g_slice_free (GetMsgAsyncHelper, helper);
	} else {
		TnyHeader *header;
		header = TNY_HEADER (tny_iterator_get_current (helper->iter)); 
		tny_folder_get_msg_async (folder, header,			  
					  get_msg_cb, helper);
		g_object_unref (G_OBJECT(header));
	}
}

void 
modest_ui_actions_on_header_selected (ModestHeaderView *folder_view, 
				      TnyHeader *header,
				      ModestMainWindow *main_window)
{
	GtkWidget *msg_preview;
	TnyFolder *folder;
	GetMsgAsyncHelper *helper;
	TnyList *list;

	g_return_if_fail (MODEST_IS_MAIN_WINDOW(main_window));
	
	msg_preview = modest_main_window_get_child_widget(main_window,
							  MODEST_WIDGET_TYPE_MSG_PREVIEW);
	if (!msg_preview)
		return;
	
	/* when there's no header, clear the msgview */
	if (!header) {
		modest_msg_view_set_message (MODEST_MSG_VIEW(msg_preview), NULL);
		return;
	}

	folder = tny_header_get_folder (TNY_HEADER(header));

	/* Create list */
	list = tny_simple_list_new ();
	tny_list_prepend (list, G_OBJECT (header));

	/* Fill helper data */
	helper = g_slice_new0 (GetMsgAsyncHelper);
	helper->window = MODEST_WINDOW (main_window);
	helper->iter = tny_list_create_iterator (list);
	helper->func = read_msg_func;

	tny_folder_get_msg_async (TNY_FOLDER(folder),
				  header, get_msg_cb,
				  helper);

	/* Frees */
	g_object_unref (G_OBJECT (folder));
}



void 
modest_ui_actions_on_header_activated (ModestHeaderView *folder_view, TnyHeader *header,
				       ModestMainWindow *main_window)
{
	ModestWindow *win;
	TnyFolder *folder = NULL;
	TnyMsg    *msg    = NULL;
	ModestWindowMgr *mgr;
	
	g_return_if_fail (MODEST_IS_MAIN_WINDOW(main_window));
	
	if (!header)
		return;

	folder = tny_header_get_folder (header);
	if (!folder) {
		g_printerr ("modest: cannot get folder for header\n");
		goto cleanup;
	}

	/* FIXME: make async?; check error  */
	msg = tny_folder_get_msg (folder, header, NULL);
	if (!msg) {
		g_printerr ("modest: cannot get msg for header\n");
		goto cleanup;
	}

	/* Look if we already have a message view for that header */	
	mgr = modest_runtime_get_window_mgr ();
	win = modest_window_mgr_find_window_by_msguid (mgr, tny_header_get_uid (header));

	/* If not, create a new window */
	if (!win) {
		gchar *account;

		account =  g_strdup(modest_window_get_active_account(MODEST_WINDOW(main_window)));
		if (!account)
			account = modest_account_mgr_get_default_account (modest_runtime_get_account_mgr());

		win = modest_msg_view_window_new (msg, account);
		modest_window_mgr_register_window (mgr, win);

		gtk_window_set_transient_for (GTK_WINDOW (win),
					      GTK_WINDOW (main_window));

		g_free (account);
	}

	gtk_widget_show_all (GTK_WIDGET(win));
	
cleanup:
	if (folder)
		g_object_unref (G_OBJECT (folder));
	if (msg)
		g_object_unref (G_OBJECT (msg));
}

void 
modest_ui_actions_on_folder_selection_changed (ModestFolderView *folder_view,
					       TnyFolder *folder, 
					       gboolean selected,
					       ModestMainWindow *main_window)
{
	gchar *txt;
	ModestConf *conf;
	GtkWidget *header_view;
	
	g_return_if_fail (MODEST_IS_MAIN_WINDOW(main_window));

	header_view = modest_main_window_get_child_widget(main_window,
							  MODEST_WIDGET_TYPE_HEADER_VIEW);
	if (!header_view)
		return;
	
	conf = modest_runtime_get_conf ();

	if (!selected) { /* the folder was unselected; save it's settings  */
		modest_widget_memory_save (conf, G_OBJECT (header_view), "header-view");
		gtk_window_set_title (GTK_WINDOW(main_window), "Modest");
		modest_header_view_set_folder (MODEST_HEADER_VIEW(header_view), NULL);
	} else {  /* the folder was selected */
		if (folder) { /* folder may be NULL */
			guint num, unread;
			gchar *title;

			num    = tny_folder_get_all_count    (folder);
			unread = tny_folder_get_unread_count (folder);
			
			title = g_strdup_printf ("Modest: %s",
						 tny_folder_get_name (folder));
			
			gtk_window_set_title (GTK_WINDOW(main_window), title);
			g_free (title);
			
			txt = g_strdup_printf (_("%d %s, %d unread"),
					       num, num==1 ? _("item") : _("items"), unread);		
			//gtk_label_set_label (GTK_LABEL(folder_info_label), txt);
			g_free (txt);
		}
		modest_header_view_set_folder (MODEST_HEADER_VIEW(header_view), folder);
		modest_widget_memory_restore (conf, G_OBJECT(header_view),
					      "header-view");
	}
}

void 
modest_ui_actions_on_item_not_found (ModestHeaderView *header_view,ModestItemType type,
				     ModestWindow *win)
{
	GtkWidget *dialog;
	gchar *txt, *item;
	gboolean online;

	item = (type == MODEST_ITEM_TYPE_FOLDER) ? "folder" : "message";
	
	if (g_main_depth > 0)	
		gdk_threads_enter ();
	online = tny_device_is_online (modest_runtime_get_device());

	if (online) {
		/* already online -- the item is simply not there... */
		dialog = gtk_message_dialog_new (GTK_WINDOW (win),
						 GTK_DIALOG_MODAL,
						 GTK_MESSAGE_WARNING,
						 GTK_BUTTONS_OK,
						 _("The %s you selected cannot be found"),
						 item);
		gtk_dialog_run (GTK_DIALOG(dialog));
	} else {
		dialog = gtk_dialog_new_with_buttons (_("Connection requested"),
						      GTK_WINDOW (win),
						      GTK_DIALOG_MODAL,
						      GTK_STOCK_CANCEL,
						      GTK_RESPONSE_REJECT,
						      GTK_STOCK_OK,
						      GTK_RESPONSE_ACCEPT,
						      NULL);
		txt = g_strdup_printf (_("This %s is not available in offline mode.\n"
					 "Do you want to get online?"), item);
		gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox), 
				    gtk_label_new (txt), FALSE, FALSE, 0);
		gtk_widget_show_all (GTK_WIDGET(GTK_DIALOG(dialog)->vbox));
		g_free (txt);

		gtk_window_set_default_size (GTK_WINDOW(dialog), 300, 300);
		if (gtk_dialog_run (GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
//			tny_device_force_online (modest_runtime_get_device());
		}
	}
	gtk_widget_destroy (dialog);
	if (g_main_depth > 0)	
		gdk_threads_leave ();
}

void
modest_ui_actions_on_msg_link_hover (ModestMsgView *msgview, const gchar* link,
				     ModestWindow *win)
{
	g_message ("%s %s", __FUNCTION__, link);
}	


void
modest_ui_actions_on_msg_link_clicked (ModestMsgView *msgview, const gchar* link,
					ModestWindow *win)
{
	modest_platform_activate_uri (link);
}

void
modest_ui_actions_on_msg_link_contextual (ModestMsgView *msgview, const gchar* link,
					  ModestWindow *win)
{
	modest_platform_show_uri_popup (link);
}

void
modest_ui_actions_on_msg_attachment_clicked (ModestMsgView *msgview, TnyMimePart *mime_part,
					     ModestWindow *win)
{
	g_message (__FUNCTION__);
	
}

void
modest_ui_actions_on_msg_recpt_activated (ModestMsgView *msgview,
					  const gchar *address,
					  ModestWindow *win)
{
	g_message ("%s %s", __FUNCTION__, address);
}

void
modest_ui_actions_on_send (GtkWidget *widget, ModestMsgEditWindow *edit_window)
{
	TnyTransportAccount *transport_account;
	ModestMailOperation *mail_operation;
	MsgData *data;
	gchar *account_name, *from;
	ModestAccountMgr *account_mgr;

	g_return_if_fail (MODEST_IS_MSG_EDIT_WINDOW(edit_window));
	
	data = modest_msg_edit_window_get_msg_data (edit_window);

	/* FIXME: Code added just for testing. The final version will
	   use the send queue provided by tinymail and some
	   classifier */
	account_mgr = modest_runtime_get_account_mgr();
	account_name = g_strdup(modest_window_get_active_account (MODEST_WINDOW(edit_window)));
	if (!account_name) 
		account_name = modest_account_mgr_get_default_account (account_mgr);
	if (!account_name) {
		g_printerr ("modest: no account found\n");
		modest_msg_edit_window_free_msg_data (edit_window, data);
		return;
	}
	transport_account =
		TNY_TRANSPORT_ACCOUNT(modest_tny_account_store_get_tny_account_by_account
				      (modest_runtime_get_account_store(),
				       account_name,
				       TNY_ACCOUNT_TYPE_TRANSPORT));
	if (!transport_account) {
		g_printerr ("modest: no transport account found for '%s'\n", account_name);
		g_free (account_name);
		modest_msg_edit_window_free_msg_data (edit_window, data);
		return;
	}
	from = modest_account_mgr_get_from_string (account_mgr, account_name);

	/* Create the mail operation */		
	mail_operation = modest_mail_operation_new ();
	modest_mail_operation_queue_add (modest_runtime_get_mail_operation_queue (), mail_operation);

	modest_mail_operation_send_new_mail (mail_operation,
					     transport_account,
					     from,
					     data->to, 
					     data->cc, 
					     data->bcc,
					     data->subject, 
					     data->plain_body, 
					     data->html_body,
					     data->attachments);
	/* Frees */
	g_free (from);
	g_free (account_name);
	g_object_unref (G_OBJECT (transport_account));
	g_object_unref (G_OBJECT (mail_operation));

	modest_msg_edit_window_free_msg_data (edit_window, data);

	/* Save settings and close the window */
	gtk_widget_destroy (GTK_WIDGET (edit_window));
}

void 
modest_ui_actions_on_toggle_bold (GtkToggleAction *action,
				  ModestMsgEditWindow *window)
{
	ModestMsgEditFormatState *format_state = NULL;

	g_return_if_fail (MODEST_IS_MSG_EDIT_WINDOW (window));
	g_return_if_fail (GTK_IS_TOGGLE_ACTION (action));

	if (modest_msg_edit_window_get_format (MODEST_MSG_EDIT_WINDOW (window)) == MODEST_MSG_EDIT_FORMAT_TEXT)
		return;

	format_state = modest_msg_edit_window_get_format_state (window);
	g_return_if_fail (format_state != NULL);

	format_state->bold = gtk_toggle_action_get_active (action);
	modest_msg_edit_window_set_format_state (window, format_state);
	g_free (format_state);
	
}

void 
modest_ui_actions_on_toggle_italics (GtkToggleAction *action,
				     ModestMsgEditWindow *window)
{
	ModestMsgEditFormatState *format_state = NULL;

	g_return_if_fail (MODEST_IS_MSG_EDIT_WINDOW (window));
	g_return_if_fail (GTK_IS_TOGGLE_ACTION (action));

	if (modest_msg_edit_window_get_format (MODEST_MSG_EDIT_WINDOW(window)) == MODEST_MSG_EDIT_FORMAT_TEXT)
		return;

	format_state = modest_msg_edit_window_get_format_state (window);
	g_return_if_fail (format_state != NULL);

	format_state->italics = gtk_toggle_action_get_active (action);
	modest_msg_edit_window_set_format_state (window, format_state);
	g_free (format_state);
	
}

void 
modest_ui_actions_on_toggle_bullets (GtkToggleAction *action,
				     ModestMsgEditWindow *window)
{
	ModestMsgEditFormatState *format_state = NULL;

	g_return_if_fail (MODEST_IS_MSG_EDIT_WINDOW (window));
	g_return_if_fail (GTK_IS_TOGGLE_ACTION (action));

	if (modest_msg_edit_window_get_format (MODEST_MSG_EDIT_WINDOW (window)) == MODEST_MSG_EDIT_FORMAT_TEXT)
		return;

	format_state = modest_msg_edit_window_get_format_state (window);
	g_return_if_fail (format_state != NULL);

	format_state->bullet = gtk_toggle_action_get_active (action);
	modest_msg_edit_window_set_format_state (window, format_state);
	g_free (format_state);
	
}

void 
modest_ui_actions_on_change_justify (GtkRadioAction *action,
				     GtkRadioAction *selected,
				     ModestMsgEditWindow *window)
{
	ModestMsgEditFormatState *format_state = NULL;
	GtkJustification value;

	g_return_if_fail (MODEST_IS_MSG_EDIT_WINDOW (window));

	if (modest_msg_edit_window_get_format (MODEST_MSG_EDIT_WINDOW(window)) == MODEST_MSG_EDIT_FORMAT_TEXT)
		return;

	value = gtk_radio_action_get_current_value (selected);

	format_state = modest_msg_edit_window_get_format_state (window);
	g_return_if_fail (format_state != NULL);

	format_state->justification = value;
	modest_msg_edit_window_set_format_state (window, format_state);
	g_free (format_state);
}

void 
modest_ui_actions_on_select_editor_color (GtkAction *action,
					  ModestMsgEditWindow *window)
{
	g_return_if_fail (MODEST_IS_MSG_EDIT_WINDOW (window));
	g_return_if_fail (GTK_IS_ACTION (action));

	if (modest_msg_edit_window_get_format (MODEST_MSG_EDIT_WINDOW(window)) == MODEST_MSG_EDIT_FORMAT_TEXT)
		return;

	modest_msg_edit_window_select_color (window);
}

void 
modest_ui_actions_on_select_editor_background_color (GtkAction *action,
						     ModestMsgEditWindow *window)
{
	g_return_if_fail (MODEST_IS_MSG_EDIT_WINDOW (window));
	g_return_if_fail (GTK_IS_ACTION (action));

	if (modest_msg_edit_window_get_format (MODEST_MSG_EDIT_WINDOW(window)) == MODEST_MSG_EDIT_FORMAT_TEXT)
		return;

	modest_msg_edit_window_select_background_color (window);
}

void 
modest_ui_actions_on_insert_image (GtkAction *action,
				   ModestMsgEditWindow *window)
{
	g_return_if_fail (MODEST_IS_MSG_EDIT_WINDOW (window));
	g_return_if_fail (GTK_IS_ACTION (action));

	if (modest_msg_edit_window_get_format (MODEST_MSG_EDIT_WINDOW(window)) == MODEST_MSG_EDIT_FORMAT_TEXT)
		return;

	modest_msg_edit_window_insert_image (window);
}

/*
 * Shows a dialog with an entry that asks for some text. The returned
 * value must be freed by the caller. The dialog window title will be
 * set to @title.
 */
static gchar *
ask_for_folder_name (GtkWindow *parent_window,
		     const gchar *title)
{
	GtkWidget *dialog, *entry;
	gchar *folder_name = NULL;

	/* Ask for folder name */
	dialog = gtk_dialog_new_with_buttons (_("New Folder Name"),
					      parent_window,
					      GTK_DIALOG_MODAL,
					      GTK_STOCK_CANCEL,
					      GTK_RESPONSE_REJECT,
					      GTK_STOCK_OK,
					      GTK_RESPONSE_ACCEPT,
					      NULL);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox), 
			    gtk_label_new(title),
			    FALSE, FALSE, 0);
		
	entry = gtk_entry_new_with_max_length (40);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox), 
			    entry,
			    TRUE, FALSE, 0);	
	
	gtk_widget_show_all (GTK_WIDGET(GTK_DIALOG(dialog)->vbox));
	
	if (gtk_dialog_run (GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)		
		folder_name = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));

	gtk_widget_destroy (dialog);

	return folder_name;
}

void 
modest_ui_actions_on_new_folder (GtkAction *action, ModestMainWindow *main_window)
{
	TnyFolder *parent_folder;
	GtkWidget *folder_view;
	
	g_return_if_fail (MODEST_IS_MAIN_WINDOW(main_window));

	folder_view = modest_main_window_get_child_widget (main_window,
							   MODEST_WIDGET_TYPE_FOLDER_VIEW);
	if (!folder_view)
		return;

	parent_folder = modest_folder_view_get_selected (MODEST_FOLDER_VIEW(folder_view));
	
	if (parent_folder) {
		gchar *folder_name;

		folder_name = ask_for_folder_name (GTK_WINDOW (main_window),
						   _("Please enter a name for the new folder"));

		if (folder_name != NULL && strlen (folder_name) > 0) {
			TnyFolder *new_folder;
			ModestMailOperation *mail_op;

			mail_op = modest_mail_operation_new ();
			modest_mail_operation_queue_add (modest_runtime_get_mail_operation_queue (), 
							 mail_op);

			new_folder = modest_mail_operation_create_folder (mail_op,
									  TNY_FOLDER_STORE (parent_folder),
									  (const gchar *) folder_name);
			if (new_folder) 
				g_object_unref (new_folder);
			g_object_unref (mail_op);
			g_free (folder_name);
		}
		g_object_unref (parent_folder);
	}
}

void 
modest_ui_actions_on_rename_folder (GtkAction *action,
				     ModestMainWindow *main_window)
{
	TnyFolder *folder;
	GtkWidget *folder_view;
	
	g_return_if_fail (MODEST_IS_MAIN_WINDOW(main_window));

	folder_view = modest_main_window_get_child_widget (main_window,
							   MODEST_WIDGET_TYPE_FOLDER_VIEW);
	if (!folder_view)
		return;
	
	folder = modest_folder_view_get_selected (MODEST_FOLDER_VIEW(folder_view));
	
	if (folder) {
		gchar *folder_name;
		folder_name = ask_for_folder_name (GTK_WINDOW (main_window),
						   _("Please enter a new name for the folder"));

		if (folder_name != NULL && strlen (folder_name) > 0) {
			ModestMailOperation *mail_op;

			mail_op = modest_mail_operation_new ();
			modest_mail_operation_queue_add (modest_runtime_get_mail_operation_queue (),
							 mail_op);

			modest_mail_operation_rename_folder (mail_op,
							     folder,
							     (const gchar *) folder_name);

			g_object_unref (mail_op);
			g_free (folder_name);
		}
		g_object_unref (folder);
	}
}

static void
delete_folder (ModestMainWindow *main_window, gboolean move_to_trash) 
{
	TnyFolder *folder;
	ModestMailOperation *mail_op;
	GtkWidget *folder_view;
	
	g_return_if_fail (MODEST_IS_MAIN_WINDOW(main_window));

	folder_view = modest_main_window_get_child_widget (main_window,
							   MODEST_WIDGET_TYPE_FOLDER_VIEW);
	if (!folder_view)
		return;

	folder = modest_folder_view_get_selected (MODEST_FOLDER_VIEW (folder_view));
	
	mail_op = modest_mail_operation_new ();
	modest_mail_operation_queue_add (modest_runtime_get_mail_operation_queue (),
					 mail_op);
	modest_mail_operation_remove_folder (mail_op, folder, move_to_trash);

	g_object_unref (G_OBJECT (mail_op));
	g_object_unref (G_OBJECT (folder));
}

void 
modest_ui_actions_on_delete_folder (GtkAction *action,
				     ModestMainWindow *main_window)
{
	g_return_if_fail (MODEST_IS_MAIN_WINDOW(main_window));

	delete_folder (main_window, FALSE);
}

void 
modest_ui_actions_on_move_folder_to_trash_folder (GtkAction *action, ModestMainWindow *main_window)
{
	g_return_if_fail (MODEST_IS_MAIN_WINDOW(main_window));
	
	delete_folder (main_window, TRUE);
}

void
modest_ui_actions_on_password_requested (TnyAccountStore *account_store, 
					 const gchar* account_name,
					 gchar **password, 
					 gboolean *cancel, 
					 gboolean *remember,
					 ModestMainWindow *main_window)
{
	gchar *txt;
	GtkWidget *dialog, *entry, *remember_pass_check;

	dialog = gtk_dialog_new_with_buttons (_("Password requested"),
					      NULL,
					      GTK_DIALOG_MODAL,
					      GTK_STOCK_CANCEL,
					      GTK_RESPONSE_REJECT,
					      GTK_STOCK_OK,
					      GTK_RESPONSE_ACCEPT,
					      NULL);
	gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(main_window));
	
	txt = g_strdup_printf (_("Please enter your password for %s"), account_name);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox), gtk_label_new(txt),
			    FALSE, FALSE, 0);
	g_free (txt);

	entry = gtk_entry_new_with_max_length (40);
	gtk_entry_set_visibility (GTK_ENTRY(entry), FALSE);
	gtk_entry_set_invisible_char (GTK_ENTRY(entry), 0x2022); /* bullet unichar */
	
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox), entry,
			    TRUE, FALSE, 0);	

	remember_pass_check = gtk_check_button_new_with_label (_("Remember password"));
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox), remember_pass_check,
			    TRUE, FALSE, 0);

	gtk_widget_show_all (GTK_WIDGET(GTK_DIALOG(dialog)->vbox));
	
	if (gtk_dialog_run (GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		*password = g_strdup (gtk_entry_get_text (GTK_ENTRY(entry)));
		*cancel   = FALSE;
	} else {
		*password = NULL;
		*cancel   = TRUE;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (remember_pass_check)))
		*remember = TRUE;
	else
		*remember = FALSE;

	gtk_widget_destroy (dialog);
}

void
modest_ui_actions_on_cut (GtkAction *action,
			  ModestWindow *window)
{
	GtkWidget *focused_widget;

	focused_widget = gtk_window_get_focus (GTK_WINDOW (window));
	if (GTK_IS_EDITABLE (focused_widget)) {
		gtk_editable_cut_clipboard (GTK_EDITABLE(focused_widget));
	} else if (GTK_IS_TEXT_VIEW (focused_widget)) {
		GtkTextBuffer *buffer;
		GtkClipboard *clipboard;

		clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (focused_widget));
		gtk_text_buffer_cut_clipboard (buffer, clipboard, TRUE);
	}
}

void
modest_ui_actions_on_copy (GtkAction *action,
			   ModestWindow *window)
{
	GtkClipboard *clipboard;
	GtkWidget *focused_widget;

	clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
	focused_widget = gtk_window_get_focus (GTK_WINDOW (window));
	if (GTK_IS_LABEL (focused_widget)) {
		gtk_clipboard_set_text (clipboard, gtk_label_get_text (GTK_LABEL (focused_widget)), -1);
	} else if (GTK_IS_EDITABLE (focused_widget)) {
		gtk_editable_copy_clipboard (GTK_EDITABLE(focused_widget));
	} else if (GTK_IS_TEXT_VIEW (focused_widget)) {
		GtkTextBuffer *buffer;

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (focused_widget));
		gtk_text_buffer_copy_clipboard (buffer, clipboard);
	}
}

void
modest_ui_actions_on_paste (GtkAction *action,
			    ModestWindow *window)
{
	GtkWidget *focused_widget;

	focused_widget = gtk_window_get_focus (GTK_WINDOW (window));
	if (GTK_IS_EDITABLE (focused_widget)) {
		gtk_editable_paste_clipboard (GTK_EDITABLE(focused_widget));
	} else if (GTK_IS_TEXT_VIEW (focused_widget)) {
		GtkTextBuffer *buffer;
		GtkClipboard *clipboard;

		clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (focused_widget));
		gtk_text_buffer_paste_clipboard (buffer, clipboard, NULL, TRUE);
	}
}

void
modest_ui_actions_on_select_all (GtkAction *action,
				 ModestWindow *window)
{
	GtkWidget *focused_widget;

	focused_widget = gtk_window_get_focus (GTK_WINDOW (window));
	if (GTK_IS_LABEL (focused_widget)) {
		gtk_label_select_region (GTK_LABEL (focused_widget), 0, -1);
	} else if (GTK_IS_EDITABLE (focused_widget)) {
		gtk_editable_select_region (GTK_EDITABLE(focused_widget), 0, -1);
	} else if (GTK_IS_TEXT_VIEW (focused_widget)) {
		GtkTextBuffer *buffer;
		GtkTextIter start, end;

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (focused_widget));
		gtk_text_buffer_get_start_iter (buffer, &start);
		gtk_text_buffer_get_end_iter (buffer, &end);
		gtk_text_buffer_select_range (buffer, &start, &end);
	}
}

void 
modest_ui_actions_on_toggle_fullscreen (GtkAction *action,
					ModestWindow *window)
{
	ModestWindowMgr *mgr;
	gboolean active;

	mgr = modest_runtime_get_window_mgr ();
	/* set/unset the application fullscreen mode */
	active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
	modest_window_mgr_set_fullscreen_mode (mgr, active);

	/* Bring the current window to the front. The above call will
	   put all the windows in fullscreen mode, so we can not be
	   sure that the last fullscreen-ed window is the current
	   one */
	gtk_window_present (GTK_WINDOW (window));
}

void
modest_ui_actions_on_change_zoom (GtkRadioAction *action,
				  GtkRadioAction *selected,
				  ModestWindow *window)
{
	gint value;

	value = gtk_radio_action_get_current_value (selected);
	if (MODEST_IS_WINDOW (window)) {
		modest_window_set_zoom (MODEST_WINDOW (window), ((gdouble)value)/100);
	}
}

static void
modest_ui_actions_message_details_cb (gpointer msg_data, 
				      gpointer helper_data)
{
	GtkWidget *dialog;
 	TnyMsg *msg = (TnyMsg *) msg_data;
	TnyHeader *header;
	GetMsgAsyncHelper *helper = (GetMsgAsyncHelper *) helper_data;
	
	header = tny_msg_get_header (msg);
	
	dialog = modest_msg_view_details_dialog_new (GTK_WINDOW (helper->window), header);
	g_object_unref (header);
	gtk_widget_show_all (dialog);

	gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);
}

void     
modest_ui_actions_on_message_details (GtkAction *action, 
				      ModestWindow *win)
{
	TnyList * headers_list;
	GetMsgAsyncHelper *helper;

	headers_list = get_selected_headers (win);
	if (!headers_list)
		return;

	helper = g_slice_new0 (GetMsgAsyncHelper);
	helper->window = win;
	helper->func = modest_ui_actions_message_details_cb;
	helper->iter = tny_list_create_iterator (headers_list);
	helper->user_data = NULL;

	if (MODEST_IS_MSG_VIEW_WINDOW (win)) {
		TnyMsg *msg;

		msg = modest_msg_view_window_get_message (MODEST_MSG_VIEW_WINDOW (win));
		if (!msg)
			return;
		else {
			modest_ui_actions_message_details_cb (msg, helper);
		}
	} else {
		/* here we should add an implementation to run the message details dialog
		   from the main window */
		g_return_if_reached ();
	}
}

void 
modest_ui_actions_on_change_fullscreen (GtkRadioAction *action,
					GtkRadioAction *selected,
					ModestWindow *window)
{
	gint value;
	ModestWindowMgr *mgr;

	value = gtk_radio_action_get_current_value (selected);
	mgr = modest_runtime_get_window_mgr ();

	if (value == 0)
		modest_window_mgr_set_fullscreen_mode (mgr, FALSE);
	else
		modest_window_mgr_set_fullscreen_mode (mgr, TRUE);

	gtk_window_present (GTK_WINDOW (window));
}
