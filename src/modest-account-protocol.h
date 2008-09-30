/* Copyright (c) 2008, Nokia Corporation
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


/* modest-account-settings.h */

#ifndef __MODEST_ACCOUNT_PROTOCOL_H__
#define __MODEST_ACCOUNT_PROTOCOL_H__

#include "widgets/modest-account-settings-dialog.h"
#include "modest-protocol.h"
#include "modest-pair.h"
#include <tny-account.h>
#include <tny-list.h>

G_BEGIN_DECLS

/* convenience macros */
#define MODEST_TYPE_ACCOUNT_PROTOCOL             (modest_account_protocol_get_type())
#define MODEST_ACCOUNT_PROTOCOL(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MODEST_TYPE_ACCOUNT_PROTOCOL,ModestAccountProtocol))
#define MODEST_ACCOUNT_PROTOCOL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MODEST_TYPE_ACCOUNT_PROTOCOL,ModestAccountProtocolClass))
#define MODEST_IS_ACCOUNT_PROTOCOL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MODEST_TYPE_ACCOUNT_PROTOCOL))
#define MODEST_IS_ACCOUNT_PROTOCOL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MODEST_TYPE_ACCOUNT_PROTOCOL))
#define MODEST_ACCOUNT_PROTOCOL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MODEST_TYPE_ACCOUNT_PROTOCOL,ModestAccountProtocolClass))

typedef struct _ModestAccountProtocol      ModestAccountProtocol;
typedef struct _ModestAccountProtocolClass ModestAccountProtocolClass;


struct _ModestAccountProtocol {
	ModestProtocol parent;
};

struct _ModestAccountProtocolClass {
	ModestProtocolClass parent_class;

	/* Virtual methods */
	TnyAccount * (*create_account) (ModestAccountProtocol *self);
	ModestAccountSettingsDialog* (*create_account_settings_dialog) (ModestAccountProtocol* self);
	ModestPairList* (*get_easysetupwizard_tabs) (ModestAccountProtocol* self);
	void (*save_settings) (ModestAccountProtocol *self, ModestAccountSettingsDialog *dialog, ModestAccountSettings *settings);
	void (*save_wizard_settings) (ModestAccountProtocol *self, GList *wizard_pages, ModestAccountSettings *settings);
};

/**
 * modest_account_protocol_get_type:
 *
 * Returns: GType of the account protocol type
 */
GType  modest_account_protocol_get_type   (void) G_GNUC_CONST;

/**
 * modest_account_protocol_new:
 *
 * creates a new instance of #ModestAccountProtocol
 *
 * Returns: a #ModestAccountProtocol
 */
ModestProtocol*    modest_account_protocol_new (const gchar *name, const gchar *display_name, 
						guint port, guint alternate_port,
						GType account_g_type);

/**
 * modest_account_protocol_get_port:
 * @self: a #ModestAccountProtocol
 *
 * get the protocol standard port
 *
 * Returns: a string
 */
guint modest_account_protocol_get_port (ModestAccountProtocol *self);

/**
 * modest_account_protocol_set_port:
 * @self: a #ModestAccountProtocol
 * @port: a #guint
 *
 * set @port as the protocol standard port
 */
void         modest_account_protocol_set_port (ModestAccountProtocol *self,
					       guint port);

/**
 * modest_account_protocol_get_alternate_port:
 * @self: a #ModestAccountProtocol
 *
 * get the protocol standard alternate_port
 *
 * Returns: a #guint
 */
guint modest_account_protocol_get_alternate_port (ModestAccountProtocol *self);

/**
 * modest_account_protocol_set_alternate_port:
 * @self: a #ModestAccountProtocol
 * @alternate_port: a #guint
 *
 * set @alternate_port as the protocol alternate port
 */
void         modest_account_protocol_set_alternate_port (ModestAccountProtocol *self,
							 guint alternate_port);

/**
 * modest_account_protocol_set_account_options:
 * @self: a #ModestAccountProtocol
 * @account_options: a #TnyList of account options and their values
 *
 * set the account options that will be passed to TnyCamelAccount for this protocol.
 * This replaces previous option lists for this protocol
 */
void modest_account_protocol_set_account_options (ModestAccountProtocol *self,
						  TnyList *account_options);

/**
 * modest_account_protocol_get_account_options:
 * @self: a #ModestAccountProtocol
 *
 * obtain the account options for this account protocol.
 *
 * Returns: a caller-owner copy of the account options list.
 */
TnyList *modest_account_protocol_get_account_options (ModestAccountProtocol *self);

/**
 * modest_account_protocol_has_custom_secure_auth_mech:
 * @self: a #ModestAccountProtocol
 * @auth_protocol_type: a #ModestProtocolType for an auth protocol
 *
 * checks whether there's a custom secure auth mech camel string for @auth_protocol_type.
 *
 * Returns: %TRUE if registered, %FALSE otherwise
 */
gboolean
modest_account_protocol_has_custom_secure_auth_mech (ModestAccountProtocol *self, ModestProtocolType auth_protocol_type);

/**
 * modest_account_protocol_get_custom_secure_auth_mech:
 * @self: a #ModestAccountProtocol
 * @auth_protocol_type: a #ModestProtocolType for an auth protocol
 *
 * obtains the secure auth mech of @auth_protocol_type in protocol. Be careful as %NULL does not imply
 * there's no custom auth mech registered (you can register %NULL). To check if it's registered, just
 * use modest_account_protocol_has_custom_secure_auth_mech().
 *
 * Returns: the secure auth mech for this auth protocol type that will be passed to camel.
 */
const gchar *
modest_account_protocol_get_custom_secure_auth_mech (ModestAccountProtocol *self, ModestProtocolType auth_protocol_type);

/**
 * modest_account_protocol_unset_custom_secure_auth_mech:
 * @self: a #ModestAccountProtocol
 * @auth_protocol_type: a #ModestProtocolType for an auth protocol
 *
 * Unsets the secure auth meth of @auth_protocol_type in protocol.
 */
void
modest_account_protocol_unset_custom_secure_auth_mech (ModestAccountProtocol *self, ModestProtocolType auth_protocol_type);

/**
 * modest_account_protocol_set_custom_secure_auth_mech:
 * @self: a #ModestAccountProtocol
 * @auth_protocol_type: a #ModestProtocolType for an auth protocol
 * @secure_auth_mech: a string or %NULL
 *
 * sets the secure auth mech of @auth_protocol_type in protocol. Be careful as %NULL does not imply
 * there's no custom auth mech registered (you can register %NULL). If you set %NULL you're regitering %NULL as the custom secure auth
 * mechanism instead of unsetting it.
 */
void
modest_account_protocol_set_custom_secure_auth_mech (ModestAccountProtocol *self, ModestProtocolType auth_protocol_type, const gchar *secure_auth_mech);

/**
 * modest_account_protocol_get_account_g_type:
 * @self: a #ModestAccountProtocol
 *
 * get the protocol type used for factoring new TnyAccount
 *
 * Returns: a #GType
 */
GType modest_account_protocol_get_account_g_type (ModestAccountProtocol *self);

/**
 * modest_account_protocol_set_account_g_type:
 * @self: a #ModestAccountProtocol
 * @account_g_type: a #GType
 *
 * set @account_g_type as the type modest_account_protocol_create_account will
 * instanciate
 */
void         modest_account_protocol_set_account_g_type (ModestAccountProtocol *self,
							 GType account_g_type);

/**
 * modest_account_protocol_create_account:
 * @self: a #ModestAccountProtocol
 *
 * create a new account instance for this protocol
 *
 * Returns: a #TnyAccount
 */
TnyAccount * modest_account_protocol_create_account (ModestAccountProtocol *self);

/**
 * modest_account_protocol_get_account_settings_dialog:
 * @self: a #ModestAccountProtocol
 * @account_name: the name of the account we're creating the dialog for
 * 
 * retrieves the account settings dialog used to setup the account
 * represented by this protocol
 * 
 * Returns: a #ModestAccountSettingsDialog
 **/
ModestAccountSettingsDialog* modest_account_protocol_get_account_settings_dialog (ModestAccountProtocol *self,
										  const gchar *account_name);

ModestPairList* modest_account_protocol_get_easysetupwizard_tabs (ModestAccountProtocol *self);

/**
 * modest_account_protocol_save_settings:
 * @self: this #ModestAccountProtocol
 * @dialog: a #ModestAccountSettingsDialog
 * @settings: the #ModestAccountSettings
 * 
 * this function stores the values held by the account settings dialog
 * in the account settings object that is passed as argument
 *
 * NOTE: this function provides a default implementation that calls
 * the save_settings method of the acocunt settings dialog. So if your
 * implementation do not do anything more just do not redefine it
 **/
void modest_account_protocol_save_settings (ModestAccountProtocol *self, 
					    ModestAccountSettingsDialog *dialog,
					    ModestAccountSettings *settings);

/**
 * modest_account_protocol_save_wizard_settings:
 * @self: this #ModestAccountProtocol
 * @wizard_pages: a list of #ModestEasysetupWizardPage
 * @settings: the #ModestAccountSettings
 * 
 * this function stores the data input by the users in the wizard in
 * the account settings object passed as argument
 **/
void modest_account_protocol_save_wizard_settings (ModestAccountProtocol *self, 
						   GList *wizard_pages,
						   ModestAccountSettings *settings);

G_END_DECLS

#endif /* __MODEST_ACCOUNT_PROTOCOL_H__ */