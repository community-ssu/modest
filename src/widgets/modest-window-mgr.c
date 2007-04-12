/* Copyright (c) 2006,2007 Nokia Corporation
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

#include <string.h>
#include "modest-window-mgr.h"
#include "widgets/modest-main-window.h"
#include "widgets/modest-msg-edit-window.h"
#include "widgets/modest-msg-view-window.h"
/* include other impl specific header files */

/* 'private'/'protected' functions */
static void modest_window_mgr_class_init (ModestWindowMgrClass *klass);
static void modest_window_mgr_init       (ModestWindowMgr *obj);
static void modest_window_mgr_finalize   (GObject *obj);

static void on_window_destroy            (ModestWindow *window, 
					  ModestWindowMgr *self);

/* list my signals  */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};

typedef struct _ModestWindowMgrPrivate ModestWindowMgrPrivate;
struct _ModestWindowMgrPrivate {
	GList *window_list;
	gboolean fullscreen_mode;
};
#define MODEST_WINDOW_MGR_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                               MODEST_TYPE_WINDOW_MGR, \
                                               ModestWindowMgrPrivate))
/* globals */
static GObjectClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

GType
modest_window_mgr_get_type (void)
{
	static GType my_type = 0;
	if (!my_type) {
		static const GTypeInfo my_info = {
			sizeof(ModestWindowMgrClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) modest_window_mgr_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(ModestWindowMgr),
			1,		/* n_preallocs */
			(GInstanceInitFunc) modest_window_mgr_init,
			NULL
		};
		my_type = g_type_register_static (G_TYPE_OBJECT,
		                                  "ModestWindowMgr",
		                                  &my_info, 0);
	}
	return my_type;
}

static void
modest_window_mgr_class_init (ModestWindowMgrClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = modest_window_mgr_finalize;

	g_type_class_add_private (gobject_class, sizeof(ModestWindowMgrPrivate));
}

static void
modest_window_mgr_init (ModestWindowMgr *obj)
{
	ModestWindowMgrPrivate *priv = MODEST_WINDOW_MGR_GET_PRIVATE(obj);

	priv->window_list = NULL;
	priv->fullscreen_mode = FALSE;
}

static void
modest_window_mgr_finalize (GObject *obj)
{
	ModestWindowMgrPrivate *priv = MODEST_WINDOW_MGR_GET_PRIVATE(obj);

	if (priv->window_list) {
		GList *iter = priv->window_list;
		/* unregister pending windows */
		while (iter) {
			modest_window_mgr_unregister_window (MODEST_WINDOW_MGR (obj), 
							     MODEST_WINDOW (iter->data));
			iter = g_list_next (iter);
		}
		g_list_free (priv->window_list);
		priv->window_list = NULL;
	}

	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

ModestWindowMgr*
modest_window_mgr_new (void)
{
	return MODEST_WINDOW_MGR(g_object_new(MODEST_TYPE_WINDOW_MGR, NULL));
}

void 
modest_window_mgr_register_window (ModestWindowMgr *self, 
				   ModestWindow *window)
{
	GList *win;
	ModestWindowMgrPrivate *priv;

	g_return_if_fail (MODEST_IS_WINDOW_MGR (self));
	g_return_if_fail (MODEST_IS_WINDOW (window));

	priv = MODEST_WINDOW_MGR_GET_PRIVATE (self);

	win = g_list_find (priv->window_list, window);
	if (win) {
		g_warning ("Trying to register an already registered window");
		return;
	}

	/* Add to list. Keep a reference to the window */
	g_object_ref (window);
	priv->window_list = g_list_prepend (priv->window_list, window);

	/* Listen to object destruction */
	g_signal_connect (window, "destroy", G_CALLBACK (on_window_destroy), self);

	/* Put into fullscreen if needed */
	if (priv->fullscreen_mode)
		gtk_window_fullscreen (GTK_WINDOW (window));
}

static void
on_window_destroy (ModestWindow *window, ModestWindowMgr *self)
{
	/* Specific stuff first */
	if (MODEST_IS_MAIN_WINDOW (window)) {
		ModestWindowMgrPrivate *priv;
		priv = MODEST_WINDOW_MGR_GET_PRIVATE (self);

		/* If more than one window already opened */
		if (g_list_length (priv->window_list) > 1) {

			/* If the user wants to close all the windows */
			if (modest_main_window_close_all (MODEST_MAIN_WINDOW (window))) {
				GList *iter = priv->window_list;
				do {
					if (iter->data != window) {
						GList *tmp = iter->next;
						on_window_destroy (MODEST_WINDOW (iter->data),
								   self);
						iter = tmp;
					} else {
						iter = g_list_next (iter);
					}
				} while (iter);
			}
		}
	} else {
		if (MODEST_IS_MSG_EDIT_WINDOW (window)) {
			/* TODO: Save currently edited message to Drafts
			   folder */
		}
	}

	/* Unregister window */
	modest_window_mgr_unregister_window (self, window);
}

void 
modest_window_mgr_unregister_window (ModestWindowMgr *self, 
				     ModestWindow *window)
{
	GList *win;
	ModestWindowMgrPrivate *priv;

	g_return_if_fail (MODEST_IS_WINDOW_MGR (self));
	g_return_if_fail (MODEST_IS_WINDOW (window));

	priv = MODEST_WINDOW_MGR_GET_PRIVATE (self);

	win = g_list_find (priv->window_list, window);
	if (!win) {
		g_warning ("Trying to unregister a window that has not being registered yet");
		return;
	}

	/* Remove from list. Remove the reference to the window */
	g_object_unref (win->data);
	priv->window_list = g_list_remove_link (priv->window_list, win);

	/* If there are no more windows registered then exit program */
	if (priv->window_list == NULL)
		gtk_main_quit ();
}

static gint
compare_msguids (ModestWindow *win,
		 const gchar *uid)
{
	const gchar *msg_uid;

	if (!MODEST_IS_MSG_VIEW_WINDOW (win))
		return 1;

	/* Get message uid from msg window */
	msg_uid = modest_msg_view_window_get_message_uid (MODEST_MSG_VIEW_WINDOW (win));
	if (msg_uid && !strcmp (msg_uid, uid))
		return 0;
	else
		return 1;
}

ModestWindow*  
modest_window_mgr_find_window_by_msguid (ModestWindowMgr *self, 
					 const gchar *msguid)
{
	ModestWindowMgrPrivate *priv;
	GList *win = NULL;

	g_return_val_if_fail (MODEST_IS_WINDOW_MGR (self), NULL);
	g_return_val_if_fail (msguid != NULL, NULL);

	priv = MODEST_WINDOW_MGR_GET_PRIVATE (self);

	/* Look for the window */
	if (priv->window_list)
		win = g_list_find_custom (priv->window_list, 
					  msguid, 
					  (GCompareFunc) compare_msguids);

	if (win)
		return win->data;
	else 
		return NULL;
}

void
modest_window_mgr_set_fullscreen_mode (ModestWindowMgr *self,
				       gboolean on)
{
	ModestWindowMgrPrivate *priv;
	GList *win = NULL;

	g_return_if_fail (MODEST_IS_WINDOW_MGR (self));

	priv = MODEST_WINDOW_MGR_GET_PRIVATE (self);

	/* If there is no change do nothing */
	if (priv->fullscreen_mode == on)
		return;

	priv->fullscreen_mode = on;

	/* Update windows */
	win = priv->window_list;
	while (win) {
		if (on)
			gtk_window_fullscreen (GTK_WINDOW (win->data));
		else
			gtk_window_unfullscreen (GTK_WINDOW (win->data));
		win = g_list_next (win);
	}
}

gboolean
modest_window_mgr_get_fullscreen_mode (ModestWindowMgr *self)
{
	ModestWindowMgrPrivate *priv;

	g_return_val_if_fail (MODEST_IS_WINDOW_MGR (self), FALSE);

	priv = MODEST_WINDOW_MGR_GET_PRIVATE (self);

	return priv->fullscreen_mode;
}
