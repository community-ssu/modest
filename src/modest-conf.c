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

#include <gconf/gconf-client.h>
#include <config.h>
#include <string.h>
#include <glib/gi18n.h>
#include "modest-conf.h"
#include "modest-marshal.h"
#include <stdio.h>

static void   modest_conf_class_init     (ModestConfClass *klass);
static void   modest_conf_init           (ModestConf *obj);
static void   modest_conf_finalize       (GObject *obj);
static void   modest_conf_on_change	 (GConfClient *client, guint conn_id,
					  GConfEntry *entry, gpointer data);
static GConfValueType modest_conf_type_to_gconf_type (ModestConfValueType value_type, 
						      GError **err);


static void
modest_conf_maemo_fake_on_change (ModestConf *conf, const gchar* key, ModestConfEvent event);

/* list my signals */
enum {
	KEY_CHANGED_SIGNAL,
	LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = {0}; 


typedef struct _ModestConfPrivate ModestConfPrivate;
struct _ModestConfPrivate {
	GConfClient *gconf_client;
};
#define MODEST_CONF_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
								     MODEST_TYPE_CONF, \
								     ModestConfPrivate))
/* globals */
static GObjectClass *parent_class = NULL;

GType
modest_conf_get_type (void)
{
	static GType my_type = 0;
	if (!my_type) {
		static const GTypeInfo my_info = {
			sizeof(ModestConfClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) modest_conf_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(ModestConf),
			1,		/* n_preallocs */
			(GInstanceInitFunc) modest_conf_init,
			NULL
		};
		my_type = g_type_register_static (G_TYPE_OBJECT,
		                                  "ModestConf",
		                                  &my_info, 0);
	}
	return my_type;
}

static void
modest_conf_class_init (ModestConfClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = (GObjectClass*) klass;

	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = modest_conf_finalize;

	g_type_class_add_private (gobject_class, sizeof(ModestConfPrivate));
	
	signals[KEY_CHANGED_SIGNAL] =
		g_signal_new ("key_changed",
			      G_TYPE_FROM_CLASS (gobject_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (ModestConfClass,key_changed),
			      NULL, NULL,
			      modest_marshal_VOID__STRING_INT,
			      G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_INT); 
}

static void
modest_conf_init (ModestConf *obj)
{
	GConfClient *conf = NULL;
	ModestConfPrivate *priv = MODEST_CONF_GET_PRIVATE(obj);
	GError *err      = NULL;

	
	priv->gconf_client = NULL;
	
	conf = gconf_client_get_default ();
	if (!conf) {
		g_printerr ("modest: could not get gconf client\n");
		return;
	}
	gconf_client_add_dir (conf,MODEST_CONF_NAMESPACE,
			      GCONF_CLIENT_PRELOAD_NONE,
			      &err);
	if (err) {
		g_printerr ("modest: error %d with gconf_client_add_dir: '%s'\n",
			    err->code, err->message);
		g_object_unref (conf);
		g_error_free (err);
		return;
	}
	
	gconf_client_notify_add (conf, MODEST_CONF_NAMESPACE,
				 modest_conf_on_change,
				 obj, NULL, &err);
	if (err) {
		g_printerr ("modest: gconf_client_notify_add error %d: '%s'\n",
			    err->code, err->message);
		g_object_unref (conf);
		g_error_free (err);
		return;
	}
	priv->gconf_client = conf; 	/* all went well! */
}

static void
modest_conf_finalize (GObject *obj)
{
	ModestConfPrivate *priv = MODEST_CONF_GET_PRIVATE(obj);
	if (priv->gconf_client) {

		gconf_client_suggest_sync (priv->gconf_client, NULL);

		g_object_unref (priv->gconf_client);
		priv->gconf_client = NULL;
	}	

	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

ModestConf*
modest_conf_new (void)
{
	ModestConf *conf;
	ModestConfPrivate *priv;
	
	conf = MODEST_CONF(g_object_new(MODEST_TYPE_CONF, NULL));
	if (!conf) {
		g_printerr ("modest: failed to init ModestConf (GConf)\n");
		return NULL;
	}

	priv = MODEST_CONF_GET_PRIVATE(conf);
	if (!priv->gconf_client) {
		g_printerr ("modest: failed to init gconf\n");
		g_object_unref (conf);
		return NULL;
	}
	
	return conf;
}


gchar*
modest_conf_get_string (ModestConf* self, const gchar* key, GError **err)
{
	ModestConfPrivate *priv;
	
	g_return_val_if_fail (self, NULL);
	g_return_val_if_fail (key,  NULL);

	priv = MODEST_CONF_GET_PRIVATE(self);
	return gconf_client_get_string (priv->gconf_client, key, err);
}


gint
modest_conf_get_int (ModestConf* self, const gchar* key, GError **err)
{
	ModestConfPrivate *priv;

	g_return_val_if_fail (self, -1);
	g_return_val_if_fail (key, -1);

	priv = MODEST_CONF_GET_PRIVATE(self);
	
	return gconf_client_get_int (priv->gconf_client, key, err);
}


gboolean
modest_conf_get_bool (ModestConf* self, const gchar* key, GError **err)
{
	ModestConfPrivate *priv;

	g_return_val_if_fail (self, FALSE);
	g_return_val_if_fail (key, FALSE);

	priv = MODEST_CONF_GET_PRIVATE(self);
	
	return gconf_client_get_bool (priv->gconf_client, key, err);
}


GSList * 
modest_conf_get_list (ModestConf* self, const gchar* key, ModestConfValueType list_type,
		      GError **err)
{
	ModestConfPrivate *priv;
	GConfValueType gconf_type;
       
	g_return_val_if_fail (self, NULL);
	g_return_val_if_fail (key,  NULL);

	priv = MODEST_CONF_GET_PRIVATE(self);

	gconf_type = modest_conf_type_to_gconf_type (list_type, err);

	return gconf_client_get_list (priv->gconf_client, key, gconf_type, err);
}




gboolean
modest_conf_set_string (ModestConf* self, const gchar* key, const gchar* val,
			GError **err)
{
	ModestConfPrivate *priv;
		
	g_return_val_if_fail (self,FALSE);
	g_return_val_if_fail (key, FALSE);
	g_return_val_if_fail (val, FALSE);
	
	priv = MODEST_CONF_GET_PRIVATE(self);

	if (!gconf_client_key_is_writable(priv->gconf_client,key,err)) {
		g_printerr ("modest: '%s' is not writable\n", key);
		return FALSE;
	}
			
	if (gconf_client_set_string (priv->gconf_client, key, val, err)) {
		modest_conf_maemo_fake_on_change (self, key, MODEST_CONF_EVENT_KEY_CHANGED);
		return TRUE;
	} else
		return FALSE;
}


gboolean
modest_conf_set_int  (ModestConf* self, const gchar* key, gint val,
		      GError **err)
{
	ModestConfPrivate *priv;
		
	g_return_val_if_fail (self,FALSE);
	g_return_val_if_fail (key, FALSE);
	
	priv = MODEST_CONF_GET_PRIVATE(self);

	if (!gconf_client_key_is_writable(priv->gconf_client,key,err)) {
		g_printerr ("modest: '%s' is not writable\n", key);
		return FALSE;
	}
			
	if (gconf_client_set_int (priv->gconf_client, key, val, err)) {
		modest_conf_maemo_fake_on_change (self, key, MODEST_CONF_EVENT_KEY_CHANGED);
		return TRUE;
	} else
		return FALSE;
}


gboolean
modest_conf_set_bool (ModestConf* self, const gchar* key, gboolean val,
		      GError **err)
{
	ModestConfPrivate *priv;
		
	g_return_val_if_fail (self,FALSE);
	g_return_val_if_fail (key, FALSE);
	
	priv = MODEST_CONF_GET_PRIVATE(self);

	if (!gconf_client_key_is_writable(priv->gconf_client,key, err)) {
		g_warning ("modest: '%s' is not writable\n", key);
		return FALSE;
	}

	if (gconf_client_set_bool (priv->gconf_client, key, val, err)) {
		modest_conf_maemo_fake_on_change (self, key, MODEST_CONF_EVENT_KEY_CHANGED);
		return TRUE;
	} else
		return FALSE;
}


gboolean
modest_conf_set_list (ModestConf* self, const gchar* key, 
		      GSList *val, ModestConfValueType list_type, 
		      GError **err)
{
	ModestConfPrivate *priv;
	GConfValueType gconf_type;
	gboolean result;
       
	g_return_val_if_fail (self, FALSE);
	g_return_val_if_fail (key, FALSE);

	priv = MODEST_CONF_GET_PRIVATE(self);

	gconf_type = modest_conf_type_to_gconf_type (list_type, err);
	if (*err)
		return FALSE;

	result = gconf_client_set_list (priv->gconf_client, key, gconf_type, val, err);
	if(*err) {
		g_warning("gconf_client_set_list() failed with key=%s. error=%s", key,
			  (*err)->message);
		result = FALSE;
	}
       
	/* TODO: Remove this, when we fix the problem: */
	/* This shows that sometimes set_list fails, while saying that it succeeded: */
	if (result) {
		const gint debug_list_length_start = g_slist_length(val);
		GSList* debug_list = gconf_client_get_list(priv->gconf_client, key, gconf_type, err);
		const gint debug_list_length_after = g_slist_length(debug_list);
	       
		if(debug_list_length_start != debug_list_length_after)
			g_warning("modest_conf_set_list(): The list length after setting is "
				  "not the same as the specified list. key=%s. "
				  "We think that we fixed this, so tell us if you see this.", key);
		g_slist_free(debug_list);
	}

	if (result)
		modest_conf_maemo_fake_on_change (self, key, MODEST_CONF_EVENT_KEY_CHANGED);
	
	return result;
}


GSList*
modest_conf_list_subkeys (ModestConf* self, const gchar* key, GError **err)
{
	ModestConfPrivate *priv;
		
	g_return_val_if_fail (self,FALSE);
	g_return_val_if_fail (key, FALSE);
	
	priv = MODEST_CONF_GET_PRIVATE(self);
			
	return gconf_client_all_dirs (priv->gconf_client,key,err);
}


gboolean
modest_conf_remove_key (ModestConf* self, const gchar* key, GError **err)
{
	ModestConfPrivate *priv;
	gboolean retval;
	
	g_return_val_if_fail (self,FALSE);
	g_return_val_if_fail (key, FALSE);
	
	priv = MODEST_CONF_GET_PRIVATE(self);
			
	retval = gconf_client_recursive_unset (priv->gconf_client,key,0,err);
	gconf_client_suggest_sync (priv->gconf_client, NULL);

	if (retval) {
		modest_conf_maemo_fake_on_change (self, key, MODEST_CONF_EVENT_KEY_UNSET);
		return TRUE;
	} else
		return FALSE;
}


gboolean
modest_conf_key_exists (ModestConf* self, const gchar* key, GError **err)
{
	ModestConfPrivate *priv;
	GConfValue *val;

	g_return_val_if_fail (self,FALSE);
	g_return_val_if_fail (key, FALSE);
	
	priv = MODEST_CONF_GET_PRIVATE(self);

	/* the fast way... */
	if (gconf_client_dir_exists (priv->gconf_client,key,err))
		return TRUE;
	
	val = gconf_client_get (priv->gconf_client, key, NULL);
	if (!val)
		return FALSE;
	else {
		gconf_value_free (val);
		return TRUE;
	}	
}


gchar*
modest_conf_key_escape (const gchar* key)
{
	g_return_val_if_fail (key, NULL);
	g_return_val_if_fail (strlen (key) > 0, g_strdup (key));
	
	return gconf_escape_key (key, strlen(key));
}


gchar*
modest_conf_key_unescape (const gchar* key)
{
	g_return_val_if_fail (key, NULL);

	return gconf_unescape_key (key, strlen(key));
}

gboolean
modest_conf_key_is_valid (const gchar* key)
{
	return gconf_valid_key (key, NULL);
}

/* hmmm... might need to make specific callback for specific keys */
static void
modest_conf_on_change (GConfClient *client, guint conn_id, GConfEntry *entry,
		       gpointer data)
{
	ModestConfEvent event;
	const gchar* key;

	event = (entry->value) ? MODEST_CONF_EVENT_KEY_CHANGED : MODEST_CONF_EVENT_KEY_UNSET;
	key    = gconf_entry_get_key (entry);

	g_signal_emit (G_OBJECT(data),
		       signals[KEY_CHANGED_SIGNAL], 0,
		       key, event);
}


static GConfValueType
modest_conf_type_to_gconf_type (ModestConfValueType value_type, GError **err)
{
	GConfValueType gconf_type;

	switch (value_type) {
	case MODEST_CONF_VALUE_INT:
		gconf_type = GCONF_VALUE_INT;
		break;
	case MODEST_CONF_VALUE_BOOL:
		gconf_type = GCONF_VALUE_BOOL;
		break;
	case MODEST_CONF_VALUE_FLOAT:
		gconf_type = GCONF_VALUE_FLOAT;
		break;
	case MODEST_CONF_VALUE_STRING:
		gconf_type = GCONF_VALUE_STRING;
		break;
	default:
		/* FIXME: use MODEST_ERROR, and error code */
		gconf_type = GCONF_VALUE_INVALID;
		g_printerr ("modest: invalid list value type %d\n", value_type);
		*err = g_error_new_literal (0, 0, "invalid list value type");
	}	
	return gconf_type;
}



////////////////////////////////////////////////////////////////////////////////
/* workaround for the b0rked dbus-gconf on maemo */
/* fires a fake change notification after 0.3 secs.
 * Might not be necessary anymore. */
#if 0
#ifdef MODEST_PLATFORM_MAEMO
typedef struct {
	GObject *obj;
	gchar   *key;
} ChangeHelper;

ChangeHelper*
change_helper_new (ModestConf *conf, const gchar *key)
{
	ChangeHelper *helper = g_slice_alloc  (sizeof(ChangeHelper));
	helper->obj  = g_object_ref(G_OBJECT(conf));
	helper->key  = g_strdup (key);
	return helper;
}

static void
change_helper_free (ChangeHelper *helper)
{
	g_object_unref (helper->obj);
	g_free (helper->key);
	helper->key = NULL;
	helper->obj = NULL;
	g_slice_free (ChangeHelper,helper);
}

static gboolean
emit_change_cb (ChangeHelper *helper)
{
	if (!helper)
		return FALSE;	
	g_signal_emit (G_OBJECT(helper->obj),signals[KEY_CHANGED_SIGNAL], 0,
		       helper->key, MODEST_CONF_EVENT_KEY_CHANGED);
	change_helper_free (helper);
	
	return FALSE;
}

static gboolean
emit_remove_cb (ChangeHelper *helper)
{
	if (!helper)
		return FALSE;
	g_signal_emit (G_OBJECT(helper->obj),signals[KEY_CHANGED_SIGNAL], 0,
		       helper->key, MODEST_CONF_EVENT_KEY_UNSET);
	change_helper_free (helper);
	
	return FALSE;
}
#endif /* MODEST_PLATFORM_MAEMO */
#endif
	
static void
modest_conf_maemo_fake_on_change (ModestConf *conf, const gchar* key, ModestConfEvent event)
{
/* hack for faster notification, might not be necessary anymore: */
#if 0
#ifdef MODEST_PLATFORM_MAEMO

	ChangeHelper *helper = change_helper_new (conf,key); 
	g_timeout_add (100, /* after 100 ms */
		       (event == MODEST_CONF_EVENT_KEY_CHANGED)
		       ? (GSourceFunc)emit_change_cb : (GSourceFunc)emit_remove_cb,
		       (gpointer)helper);
#endif /* MODEST_PLATFORM_MAEMO */
#endif 
}
//////////////////////////////////////////////////////////////////////////////////
