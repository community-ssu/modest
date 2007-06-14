/* Copyright (c) 2007, Nokia Corporation
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
#include <glib/gi18n.h>

#include <modest-tny-local-folders-account.h>
#include <modest-tny-outbox-account.h>
#include <modest-tny-folder.h>
#include <tny-merge-folder.h>
#include <tny-simple-list.h>

#include <string.h>
#include <stdio.h>

G_DEFINE_TYPE (ModestTnyLocalFoldersAccount, 
	modest_tny_local_folders_account, 
	TNY_TYPE_CAMEL_STORE_ACCOUNT);

#define TNY_LOCAL_FOLDERS_ACCOUNT_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MODEST_TYPE_TNY_LOCAL_FOLDERS_ACCOUNT, ModestTnyLocalFoldersAccountPrivate))

typedef struct _ModestTnyLocalFoldersAccountPrivate ModestTnyLocalFoldersAccountPrivate;

struct _ModestTnyLocalFoldersAccountPrivate
{
	GSList *list_extra_folders;
};

static void
modest_tny_local_folders_account_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (modest_tny_local_folders_account_parent_class)->dispose)
    G_OBJECT_CLASS (modest_tny_local_folders_account_parent_class)->dispose (object);
}


static void
modest_tny_local_folders_account_remove_all_extra_folders (ModestTnyLocalFoldersAccount *store)
{
	ModestTnyLocalFoldersAccountPrivate *priv = 
		TNY_LOCAL_FOLDERS_ACCOUNT_GET_PRIVATE (store);
		
	GSList *iter = priv->list_extra_folders;
	while (iter)
	{
		TnyFolder *folder = (TnyFolder*)iter->data;
	 	if (folder) {
			g_object_unref (folder);
			iter->data = NULL;
		}
			
		iter = g_slist_next (iter);
	}
	
	g_slist_free (priv->list_extra_folders);
	priv->list_extra_folders = NULL;
}

static void
modest_tny_local_folders_account_finalize (GObject *object)
{
 	G_OBJECT_CLASS (modest_tny_local_folders_account_parent_class)->finalize (object);
  
 	ModestTnyLocalFoldersAccount *self = 
 		MODEST_TNY_LOCAL_FOLDERS_ACCOUNT (object);
		
	modest_tny_local_folders_account_remove_all_extra_folders (self);
}

static void
get_folders (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err);

static void 
get_folders_async (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, TnyStatusCallback status_callback, gpointer user_data);

static void
modest_tny_local_folders_account_class_init (ModestTnyLocalFoldersAccountClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	g_type_class_add_private (klass, sizeof (ModestTnyLocalFoldersAccountPrivate));
	
	object_class->dispose = modest_tny_local_folders_account_dispose;
	object_class->finalize = modest_tny_local_folders_account_finalize;
	  
	/* Override virtual functions from the parent class: */
	TNY_CAMEL_STORE_ACCOUNT_CLASS(klass)->get_folders_func = get_folders;
	TNY_CAMEL_STORE_ACCOUNT_CLASS(klass)->get_folders_async_func = get_folders_async;
}

static void
modest_tny_local_folders_account_init (ModestTnyLocalFoldersAccount *self)
{
}

ModestTnyLocalFoldersAccount*
modest_tny_local_folders_account_new (void)
{
  return g_object_new (MODEST_TYPE_TNY_LOCAL_FOLDERS_ACCOUNT, NULL);
}

void
modest_tny_local_folders_account_add_extra_folder (ModestTnyLocalFoldersAccount *store, 
	TnyFolder *folder)
{
	ModestTnyLocalFoldersAccountPrivate *priv = 
		TNY_LOCAL_FOLDERS_ACCOUNT_GET_PRIVATE (store);
		
	/* Check that it isn't already in the list: */
	GSList *exists = g_slist_find (priv->list_extra_folders, folder);
	if (exists)
		return;
		
	/* Add it: */
	/* The reference is released in finalize: */
	priv->list_extra_folders = g_slist_append (priv->list_extra_folders, folder);
	g_object_ref (folder);
}

static void
get_folders (TnyFolderStore *self, TnyList *list, TnyFolderStoreQuery *query, GError **err)
{
	ModestTnyLocalFoldersAccountPrivate *priv = 
		TNY_LOCAL_FOLDERS_ACCOUNT_GET_PRIVATE (self);
		
	/* Call the base class implementation: */
	TnyCamelStoreAccountClass *parent_class = g_type_class_peek_parent (
		MODEST_TNY_LOCAL_FOLDERS_ACCOUNT_GET_CLASS (self));
	parent_class->get_folders_func (self, list, query, err);
	
	/* Add our extra folders: */
	GSList *iter = priv->list_extra_folders;
	while (iter)
	{
		TnyFolder *folder = TNY_FOLDER (iter->data);
		if (folder)
			tny_list_append (list, G_OBJECT (folder));
	  		
		iter = g_slist_next (iter);
	}
}

static void 
get_folders_async (TnyFolderStore *self, TnyList *list, TnyGetFoldersCallback callback, TnyFolderStoreQuery *query, TnyStatusCallback status_callback, gpointer user_data)
{
	/* Call the base class implementation: */
	TnyCamelStoreAccountClass *parent_class = g_type_class_peek_parent (
		MODEST_TNY_LOCAL_FOLDERS_ACCOUNT_GET_CLASS (self));
	parent_class->get_folders_async_func (self, list, callback, query, status_callback, user_data);
}

static void
add_account_folders_to_merged_folder (TnyAccount *account, TnyMergeFolder* merge_folder)
{
	const gchar* account_id = tny_account_get_id (account);
	const gboolean is_actual_local_folders_account = account_id && 
		(strcmp (account_id, MODEST_LOCAL_FOLDERS_ACCOUNT_ID) == 0);
		
	TnyList *list_outbox_folders = tny_simple_list_new ();
	tny_folder_store_get_folders (TNY_FOLDER_STORE (account), 
		list_outbox_folders, NULL, NULL);
		
	TnyIterator*  iter =  tny_list_create_iterator (list_outbox_folders);
	while (!tny_iterator_is_done (iter))
	{
		TnyFolder *folder = TNY_FOLDER (tny_iterator_get_current (iter));
		
		if (folder) {
			gboolean add = TRUE;
			/* TODO: Do not add outboxes that are inside local-folders/, 
			 * because these are just left-over from earlier Modest versions 
			 * that put the outbox there: */
			if (is_actual_local_folders_account) {
				const TnyFolderType type = modest_tny_folder_get_local_folder_type (folder);
				if (type == TNY_FOLDER_TYPE_OUTBOX) {
					add = FALSE;
				}
			}
			
			if (add)
				tny_merge_folder_add_folder (merge_folder, folder);
				
			g_object_unref (folder);	
		}
		
		tny_iterator_next (iter);
	}
	
	g_object_unref (list_outbox_folders);
}

void modest_tny_local_folders_account_add_merged_outbox_folders (ModestTnyLocalFoldersAccount *self, 
	GSList *accounts)
{
	modest_tny_local_folders_account_remove_all_extra_folders (self);
	
	/* All per-account outbox folders are merged into one folders
	 * so that they appear as one outbox to the user: */
	TnyMergeFolder *merged_outbox = TNY_MERGE_FOLDER (tny_merge_folder_new());
	
	GSList *iter = accounts;
	while (iter)
	{
		TnyAccount *account = TNY_ACCOUNT (iter->data);
		if (account) {
			/* Add both outbox account and local-folders account folders
			 * to our one combined account:
			 */
			if (MODEST_IS_TNY_OUTBOX_ACCOUNT (account)) {
				/* Add the folder to the merged folder.
				 * We will add it later to the virtual local-folders store: */
				add_account_folders_to_merged_folder (account, merged_outbox);
			}
		}
	   
		iter = g_slist_next (iter);
	}
	
	/* Add the merged outbox folder to the virtual local-folders store: */
	/* printf ("Debug: %s: adding merged outbox.\n", __FUNCTION__); */
	modest_tny_local_folders_account_add_extra_folder (self, TNY_FOLDER(merged_outbox));
	g_object_unref (merged_outbox);
	merged_outbox = NULL;
}


