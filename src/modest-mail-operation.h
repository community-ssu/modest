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

#ifndef __MODEST_MAIL_OPERATION_H__
#define __MODEST_MAIL_OPERATION_H__

#include <tny-transport-account.h>
#include "modest-tny-attachment.h"
/* other include files */

G_BEGIN_DECLS

/* convenience macros */
#define MODEST_TYPE_MAIL_OPERATION             (modest_mail_operation_get_type())
#define MODEST_MAIL_OPERATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MODEST_TYPE_MAIL_OPERATION,ModestMailOperation))
#define MODEST_MAIL_OPERATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MODEST_TYPE_MAIL_OPERATION,GObject))
#define MODEST_IS_MAIL_OPERATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MODEST_TYPE_MAIL_OPERATION))
#define MODEST_IS_MAIL_OPERATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MODEST_TYPE_MAIL_OPERATION))
#define MODEST_MAIL_OPERATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MODEST_TYPE_MAIL_OPERATION,ModestMailOperationClass))

typedef struct _ModestMailOperation      ModestMailOperation;
typedef struct _ModestMailOperationClass ModestMailOperationClass;

/* typedef enum _ModestMailOperationForwardType ModestMailOperationForwardType; */
/* typedef enum _ModestMailOperationReplyType   ModestMailOperationReplyType; */
/* typedef enum _ModestMailOperationReplyMode   ModestMailOperationReplyMode; */
/* typedef enum _ModestMailOperationStatus      ModestMailOperationStatus; */

typedef enum _ModestMailOperationForwardType {
	MODEST_MAIL_OPERATION_FORWARD_TYPE_INLINE = 1,
	MODEST_MAIL_OPERATION_FORWARD_TYPE_ATTACHMENT
} ModestMailOperationForwardType;

typedef enum _ModestMailOperationReplyType {
	MODEST_MAIL_OPERATION_REPLY_TYPE_CITE = 1,
	MODEST_MAIL_OPERATION_REPLY_TYPE_QUOTE
} ModestMailOperationReplyType;

typedef enum _ModestMailOperationReplyMode {
	MODEST_MAIL_OPERATION_REPLY_MODE_SENDER,
	MODEST_MAIL_OPERATION_REPLY_MODE_LIST,
	MODEST_MAIL_OPERATION_REPLY_MODE_ALL
} ModestMailOperationReplyMode;

typedef enum _ModestMailOperationStatus {
	MODEST_MAIL_OPERATION_STATUS_INVALID,
	MODEST_MAIL_OPERATION_STATUS_SUCCESS,
	MODEST_MAIL_OPERATION_STATUS_FINISHED_WITH_ERRORS,
	MODEST_MAIL_OPERATION_STATUS_FAILED,
	MODEST_MAIL_OPERATION_STATUS_IN_PROGRESS,
	MODEST_MAIL_OPERATION_STATUS_CANCELED
} ModestMailOperationStatus;

struct _ModestMailOperation {
	 GObject parent;
	/* insert public members, if any */
};

struct _ModestMailOperationClass {
	GObjectClass parent_class;

	/* Signals */
	void (*progress_changed) (ModestMailOperation *mail_op, gpointer user_data);
};

/* member functions */
GType        modest_mail_operation_get_type    (void) G_GNUC_CONST;

/* typical parameter-less _new function */
ModestMailOperation*    modest_mail_operation_new         (void);

/* fill in other public functions, eg.: */
void    modest_mail_operation_send_mail       (ModestMailOperation *mail_op,
					       TnyTransportAccount *transport_account,
					       TnyMsg* msg);

void    modest_mail_operation_send_new_mail   (ModestMailOperation *mail_op,
					       TnyTransportAccount *transport_account,
					       const gchar *from,
					       const gchar *to,
					       const gchar *cc,
					       const gchar *bcc,
					       const gchar *subject,
					       const gchar *body,
					       const GList *attachments_list);

TnyMsg* modest_mail_operation_create_forward_mail (TnyMsg *msg, 
						   const gchar *from,
						   ModestMailOperationForwardType forward_type);

TnyMsg* modest_mail_operation_create_reply_mail    (TnyMsg *msg, 
						    const gchar *from,
						    ModestMailOperationReplyType reply_type,
						    ModestMailOperationReplyMode reply_mode);

gboolean      modest_mail_operation_update_account (ModestMailOperation *mail_op,
						    TnyStoreAccount *store_account);

/* Functions that perform store operations */
TnyFolder*    modest_mail_operation_create_folder  (ModestMailOperation *mail_op,
						    TnyFolderStore *parent,
						    const gchar *name);

void          modest_mail_operation_remove_folder  (ModestMailOperation *mail_op,
						    TnyFolder *folder,
						    gboolean remove_to_trash);

void          modest_mail_operation_rename_folder  (ModestMailOperation *mail_op,
						    TnyFolder *folder, 
						    const gchar *name);

void          modest_mail_operation_move_folder    (ModestMailOperation *mail_op,
						    TnyFolder *folder, 
						    TnyFolderStore *parent);

void          modest_mail_operation_copy_folder    (ModestMailOperation *mail_op,
						    TnyFolder *folder, 
						    TnyFolderStore *parent);

/* Functions that performs msg operations */

void          modest_mail_operation_copy_msg       (ModestMailOperation *mail_op,
						    TnyHeader *header, 
						    TnyFolder *folder);

void          modest_mail_operation_move_msg       (ModestMailOperation *mail_op,
						    TnyHeader *header, 
						    TnyFolder *folder);

void          modest_mail_operation_remove_msg     (ModestMailOperation *mail_op,
						    TnyHeader *header,
						    gboolean remove_to_trash);

/* Functions to control mail operations */
guint     modest_mail_operation_get_task_done      (ModestMailOperation *mail_op);

guint     modest_mail_operation_get_task_total     (ModestMailOperation *mail_op);


gboolean                  modest_mail_operation_is_finished (ModestMailOperation *mail_op);

ModestMailOperationStatus modest_mail_operation_get_status  (ModestMailOperation *mail_op);

const GError*             modest_mail_operation_get_error   (ModestMailOperation *mail_op);

void                      modest_mail_operation_cancel      (ModestMailOperation *mail_op);

G_END_DECLS

#endif /* __MODEST_MAIL_OPERATION_H__ */

