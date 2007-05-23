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

#include "libmodest-dbus-client.h"
#include <dbus_api/modest-dbus-api.h> /* For the API strings. */

gboolean
libmodest_dbus_client_send_mail (osso_context_t *osso_context, const gchar *to, const gchar *cc, 
	const gchar *bcc, const gchar* subject, const gchar* body, GSList *attachments)
{
	osso_rpc_t retval;
	const osso_return_t ret = osso_rpc_run_with_defaults(osso_context, 
		   MODEST_DBUS_NAME, 
		   MODEST_DBUS_METHOD_SEND_MAIL, &retval, 
		   DBUS_TYPE_STRING, to, 
		   DBUS_TYPE_STRING, cc, 
		   DBUS_TYPE_STRING, bcc, 
		   DBUS_TYPE_STRING, subject, 
		   DBUS_TYPE_STRING, body, 
		   DBUS_TYPE_INVALID);
		
	if (ret != OSSO_OK) {
		printf("debug: osso_rpc_run() failed.\n");
		return FALSE;
	} else {
		printf("debug: osso_rpc_run() succeeded.\n");
	}
	
	osso_rpc_free_val(&retval);
	
	return TRUE;
}
	
gboolean 
libmodest_dbus_client_mail_to (osso_context_t *osso_context, const gchar *mailto_uri)
{
	osso_rpc_t retval;
	const osso_return_t ret = osso_rpc_run_with_defaults(osso_context, 
		   MODEST_DBUS_NAME, 
		   MODEST_DBUS_METHOD_MAIL_TO, &retval, 
		   DBUS_TYPE_STRING, mailto_uri, 
		   DBUS_TYPE_INVALID);
		
	if (ret != OSSO_OK) {
		printf("debug: osso_rpc_run() failed.\n");
		return FALSE;
	} else {
		printf("debug: osso_rpc_run() succeeded.\n");
	}
	
	osso_rpc_free_val(&retval);
	
	return TRUE;
}

gboolean
libmodest_dbus_client_compose_mail (osso_context_t *osso_context, const gchar *to, const gchar *cc, 
	const gchar *bcc, const gchar* subject, const gchar* body, GSList *attachments)
{
	osso_rpc_t retval;
	gchar *attachments_str = NULL;
	gchar *tmp = NULL;
	GSList *next = NULL;
	
	attachments_str = g_strdup( (gchar *) attachments->data );
	
	for (next = g_slist_next(attachments); next != NULL; next = g_slist_next(next))
	{
		tmp = g_strconcat(attachments_str, ",", (gchar *) (next->data), NULL);
		g_free(attachments_str);
		attachments_str = tmp;
		if (attachments_str == NULL) {
			return OSSO_ERROR;
		}
	}

	const osso_return_t ret = osso_rpc_run_with_defaults(osso_context, 
		   MODEST_DBUS_NAME, 
		   MODEST_DBUS_METHOD_COMPOSE_MAIL, &retval, 
		   DBUS_TYPE_STRING, to, 
		   DBUS_TYPE_STRING, cc, 
		   DBUS_TYPE_STRING, bcc, 
		   DBUS_TYPE_STRING, subject, 
		   DBUS_TYPE_STRING, body,
		   DBUS_TYPE_STRING, attachments_str,
		   DBUS_TYPE_INVALID);
		
	if (ret != OSSO_OK) {
		printf("debug: osso_rpc_run() failed.\n");
		return FALSE;
	} else {
		printf("debug: osso_rpc_run() succeeded.\n");
	}
	
	osso_rpc_free_val(&retval);
	
	return TRUE;
}

gboolean 
libmodest_dbus_client_open_message (osso_context_t *osso_context, const gchar *mail_uri)
{
	osso_rpc_t retval;
	const osso_return_t ret = osso_rpc_run_with_defaults(osso_context, 
		   MODEST_DBUS_NAME, 
		   MODEST_DBUS_METHOD_OPEN_MESSAGE, &retval, 
		   DBUS_TYPE_STRING, mail_uri, 
		   DBUS_TYPE_INVALID);
		
	if (ret != OSSO_OK) {
		printf("debug: osso_rpc_run() failed.\n");
		return FALSE;
	} else {
		printf("debug: osso_rpc_run() succeeded.\n");
	}
	
	osso_rpc_free_val(&retval);
	
	return TRUE;
}

gboolean 
libmodest_dbus_client_send_and_receive (osso_context_t *osso_context)
{
	osso_rpc_t retval;
	const osso_return_t ret = osso_rpc_run_with_defaults(osso_context, 
		   MODEST_DBUS_NAME, 
		   MODEST_DBUS_METHOD_SEND_RECEIVE, &retval, 
		   DBUS_TYPE_INVALID);
		
	if (ret != OSSO_OK) {
		printf("debug: osso_rpc_run() failed.\n");
		return FALSE;
	} else {
		printf("debug: osso_rpc_run() succeeded.\n");
	}
	
	osso_rpc_free_val(&retval);
	
	return TRUE;
}
