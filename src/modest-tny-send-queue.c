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


#include <modest-tny-send-queue.h>
#include <tny-simple-list.h>
#include <tny-iterator.h>
#include <tny-folder.h>
#include <tny-camel-msg.h>
#include <modest-tny-account.h>

/* 'private'/'protected' functions */
static void modest_tny_send_queue_class_init (ModestTnySendQueueClass *klass);
static void modest_tny_send_queue_finalize   (GObject *obj);
static void modest_tny_send_queue_instance_init (GTypeInstance *instance, gpointer g_class);

/* list my signals  */
enum {
	/* MY_SIGNAL_1, */
	/* MY_SIGNAL_2, */
	LAST_SIGNAL
};

typedef struct _ModestTnySendQueuePrivate ModestTnySendQueuePrivate;
struct _ModestTnySendQueuePrivate {
	/* empty */
};
#define MODEST_TNY_SEND_QUEUE_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                                   MODEST_TYPE_TNY_SEND_QUEUE, \
                                                   ModestTnySendQueuePrivate))
/* globals */
static TnyCamelSendQueueClass *parent_class = NULL;

/* uncomment the following if you have defined any signals */
/* static guint signals[LAST_SIGNAL] = {0}; */

/*
 * this thread actually tries to send all the mails in the outbox
 */


static void
modest_tny_send_queue_cancel (TnySendQueue *self, gboolean remove, GError **err)
{	
	TNY_CAMEL_SEND_QUEUE_CLASS(parent_class)->cancel_func (self, remove, err); /* FIXME */
}

static void
modest_tny_send_queue_add (TnySendQueue *self, TnyMsg *msg, GError **err)
{
	ModestTnySendQueuePrivate *priv; 
	
	g_return_if_fail (TNY_IS_SEND_QUEUE(self));
	g_return_if_fail (TNY_IS_CAMEL_MSG(msg));
	
	priv = MODEST_TNY_SEND_QUEUE_GET_PRIVATE (self);

	/* FIXME: do something smart here... */
	
	TNY_CAMEL_SEND_QUEUE_CLASS(parent_class)->add_func (self, msg, err); /* FIXME */
}

static TnyFolder*
modest_tny_send_queue_get_sentbox (TnySendQueue *self)
{
	TnyFolder *folder;
	TnyCamelTransportAccount *account;

	g_return_val_if_fail (self, NULL);

	account = tny_camel_send_queue_get_transport_account (TNY_CAMEL_SEND_QUEUE(self));
	if (!account) {
		g_printerr ("modest: no account for send queue\n");
		return NULL;
	}
	folder  = modest_tny_account_get_special_folder (TNY_ACCOUNT(account),
							 TNY_FOLDER_TYPE_SENT);
	g_object_unref (G_OBJECT(account));

	return folder;
}


static TnyFolder*
modest_tny_send_queue_get_outbox (TnySendQueue *self)
{
	TnyFolder *folder;
	TnyCamelTransportAccount *account;

	g_return_val_if_fail (self, NULL);

	account = tny_camel_send_queue_get_transport_account (TNY_CAMEL_SEND_QUEUE(self));
	if (!account) {
		g_printerr ("modest: no account for send queue\n");
		return NULL;
	}
	folder  = modest_tny_account_get_special_folder (TNY_ACCOUNT(account),
							 TNY_FOLDER_TYPE_OUTBOX);
	g_object_unref (G_OBJECT(account));

	return folder;
}


GType
modest_tny_send_queue_get_type (void)
{
	static GType my_type = 0;

	if (my_type == 0) {
		static const GTypeInfo my_info = {
			sizeof(ModestTnySendQueueClass),
			NULL,		/* base init */
			NULL,		/* base finalize */
			(GClassInitFunc) modest_tny_send_queue_class_init,
			NULL,		/* class finalize */
			NULL,		/* class data */
			sizeof(ModestTnySendQueue),
			0,		/* n_preallocs */
			(GInstanceInitFunc) modest_tny_send_queue_instance_init,
			NULL
		};
		my_type = g_type_register_static (TNY_TYPE_CAMEL_SEND_QUEUE,
						  "ModestTnySendQueue",
						  &my_info, 0);
	}
	return my_type;
}


static void
modest_tny_send_queue_class_init (ModestTnySendQueueClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = (GObjectClass*) klass;
	
	parent_class            = g_type_class_peek_parent (klass);
	gobject_class->finalize = modest_tny_send_queue_finalize;

	TNY_CAMEL_SEND_QUEUE_CLASS(klass)->add_func         = modest_tny_send_queue_add;
	TNY_CAMEL_SEND_QUEUE_CLASS(klass)->get_outbox_func  = modest_tny_send_queue_get_outbox;
        TNY_CAMEL_SEND_QUEUE_CLASS(klass)->get_sentbox_func = modest_tny_send_queue_get_sentbox;
        TNY_CAMEL_SEND_QUEUE_CLASS(klass)->cancel_func      = modest_tny_send_queue_cancel;

	g_type_class_add_private (gobject_class, sizeof(ModestTnySendQueuePrivate));
}

static void
modest_tny_send_queue_instance_init (GTypeInstance *instance, gpointer g_class)
{
   
}

static void
modest_tny_send_queue_finalize (GObject *obj)
{
	G_OBJECT_CLASS(parent_class)->finalize (obj);
}

ModestTnySendQueue*
modest_tny_send_queue_new (TnyCamelTransportAccount *account)
{
	ModestTnySendQueue *self;
	ModestTnySendQueuePrivate *priv;
	
	g_return_val_if_fail (TNY_IS_CAMEL_TRANSPORT_ACCOUNT(account), NULL);
	
	self = MODEST_TNY_SEND_QUEUE(g_object_new(MODEST_TYPE_TNY_SEND_QUEUE, NULL));
	priv = MODEST_TNY_SEND_QUEUE_GET_PRIVATE(self);
	
	tny_camel_send_queue_set_transport_account (TNY_CAMEL_SEND_QUEUE(self),
						    account); 
	return self;
}



void
modest_tny_send_queue_try_to_send (ModestTnySendQueue* self)
{
	/* TODO: Rename this to tny_camel_send_queue_try_to_send() in tinymail 
	and check that it works, without creating a second worker. */
/* 	tny_camel_send_queue_flush (TNY_CAMEL_SEND_QUEUE(self)); */
}
