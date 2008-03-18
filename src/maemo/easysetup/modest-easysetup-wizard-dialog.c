
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
 

#include "modest-easysetup-wizard-dialog.h"
#include <glib/gi18n.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkseparator.h>
#include "maemo/easysetup/modest-easysetup-country-combo-box.h"
#include "maemo/easysetup/modest-easysetup-provider-combo-box.h"
#include "maemo/easysetup/modest-easysetup-servertype-combo-box.h"
#include "widgets/modest-serversecurity-combo-box.h"
#include "widgets/modest-secureauth-combo-box.h"
#include "widgets/modest-validating-entry.h"
#include "modest-text-utils.h"
#include "modest-account-mgr.h"
#include "modest-account-mgr-helpers.h"
#include "modest-runtime.h" /* For modest_runtime_get_account_mgr(). */
#include "maemo/modest-connection-specific-smtp-window.h"
#include "widgets/modest-ui-constants.h"
#include "widgets/modest-account-settings-dialog.h"
#include "maemo/modest-maemo-utils.h"
#include "modest-utils.h"
#include <gconf/gconf-client.h>
#include <string.h> /* For strlen(). */
#include "maemo/modest-hildon-includes.h"

/* Include config.h so that _() works: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

G_DEFINE_TYPE (ModestEasysetupWizardDialog, modest_easysetup_wizard_dialog, MODEST_TYPE_WIZARD_DIALOG);

#define MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
   						    MODEST_TYPE_EASYSETUP_WIZARD_DIALOG, \
						    ModestEasysetupWizardDialogPrivate))

typedef struct _ModestEasysetupWizardDialogPrivate ModestEasysetupWizardDialogPrivate;


typedef enum {
	MODEST_EASYSETUP_WIZARD_DIALOG_INCOMING_CHANGED = 0x01,
	MODEST_EASYSETUP_WIZARD_DIALOG_OUTGOING_CHANGED = 0x02
} ModestEasysetupWizardDialogServerChanges;

struct _ModestEasysetupWizardDialogPrivate
{
	ModestPresets *presets;

	/* Remember what fields the user edited manually to not prefill them
	 * again. */
	ModestEasysetupWizardDialogServerChanges server_changes;
	
	/* Check if the user changes a field to show a confirmation dialog */
	gboolean dirty;

	/* If we have a pending load of settings or not. */
	gboolean pending_load_settings;
	
	/* Used by derived widgets to query existing accounts,
	 * and to create new accounts: */
	ModestAccountMgr *account_manager;
	ModestAccountSettings *settings;
	
	/* notebook pages: */
	GtkWidget *page_welcome;
	
	GtkWidget *page_account_details;
	GtkWidget *combo_account_country;
	GtkWidget *combo_account_serviceprovider;
	GtkWidget *entry_account_title;
	
	GtkWidget *page_user_details;
	GtkWidget *entry_user_name;
	GtkWidget *entry_user_username;
	GtkWidget *entry_user_password;
	GtkWidget *entry_user_email;
	
	GtkWidget *page_complete_easysetup;
	
	GtkWidget *page_custom_incoming;
	GtkWidget *combo_incoming_servertype;
	GtkWidget *caption_incoming;
	GtkWidget *entry_incomingserver;
	GtkWidget *combo_incoming_security;
	GtkWidget *checkbox_incoming_auth;

	GtkWidget *page_custom_outgoing;
	GtkWidget *entry_outgoingserver;
	GtkWidget *combo_outgoing_security;
	GtkWidget *combo_outgoing_auth;
	GtkWidget *checkbox_outgoing_smtp_specific;
	GtkWidget *button_outgoing_smtp_servers;
	
	GtkWidget *page_complete_customsetup;
	
	GtkWidget *specific_window;
};

static void save_to_settings (ModestEasysetupWizardDialog *self);


static gboolean
on_delete_event (GtkWidget *widget,
		 GdkEvent *event,
		 ModestEasysetupWizardDialog *wizard)
{
	gtk_dialog_response (GTK_DIALOG (wizard), GTK_RESPONSE_CANCEL);
	return TRUE;
}

static void
on_easysetup_changed(GtkWidget* widget, ModestEasysetupWizardDialog* wizard)
{
	ModestEasysetupWizardDialogPrivate* priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(wizard);
	g_return_if_fail (priv != NULL);
	priv->dirty = TRUE;
}

static void
on_incoming_security_changed(GtkWidget* widget, ModestEasysetupWizardDialog* wizard)
{
	ModestEasysetupWizardDialogPrivate* priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(wizard);
	ModestConnectionProtocol protocol_security_incoming;

	g_return_if_fail (priv != NULL);
	protocol_security_incoming = modest_serversecurity_combo_box_get_active_serversecurity (
		MODEST_SERVERSECURITY_COMBO_BOX (priv->combo_incoming_security));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbox_incoming_auth), modest_protocol_info_is_secure (protocol_security_incoming));
	gtk_widget_set_sensitive (priv->checkbox_incoming_auth, !modest_protocol_info_is_secure (protocol_security_incoming));
	
	on_easysetup_changed (widget, wizard);
}

static void
modest_easysetup_wizard_dialog_get_property (GObject *object, guint property_id,
					     GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
modest_easysetup_wizard_dialog_set_property (GObject *object, guint property_id,
					     const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
modest_easysetup_wizard_dialog_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (modest_easysetup_wizard_dialog_parent_class)->dispose)
		G_OBJECT_CLASS (modest_easysetup_wizard_dialog_parent_class)->dispose (object);
}

static void
modest_easysetup_wizard_dialog_finalize (GObject *object)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (object);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	
	if (priv->account_manager)
		g_object_unref (G_OBJECT (priv->account_manager));
		
	if (priv->presets)
		modest_presets_destroy (priv->presets);
		
	if (priv->specific_window)
	 	gtk_widget_destroy (priv->specific_window);

	if (priv->settings)
		g_object_unref (priv->settings);
	 	
	G_OBJECT_CLASS (modest_easysetup_wizard_dialog_parent_class)->finalize (object);
}

static void
show_error (GtkWidget *parent_widget, const gchar* text);

static void
create_subsequent_easysetup_pages (ModestEasysetupWizardDialog *self);

static void
set_default_custom_servernames(ModestEasysetupWizardDialog *dialog);

static void on_combo_servertype_changed(GtkComboBox *combobox, gpointer user_data);

static gint get_serverport_incoming(ModestPresetsServerType servertype_incoming,
                                    ModestPresetsSecurity security_incoming)
{
	int serverport_incoming = 0;
		/* We don't check for SMTP here as that is impossible for an incoming server. */
		if (servertype_incoming == MODEST_PRESETS_SERVER_TYPE_IMAP) {
			serverport_incoming =
				(security_incoming & MODEST_PRESETS_SECURITY_SECURE_INCOMING_ALTERNATE_PORT) ? 993 : 143; 
		} else if (servertype_incoming == MODEST_PRESETS_SERVER_TYPE_POP) {
			serverport_incoming =
				(security_incoming & MODEST_PRESETS_SECURITY_SECURE_INCOMING_ALTERNATE_PORT) ? 995 : 110; 
		}
	return serverport_incoming;
}

static GList* 
check_for_supported_auth_methods (ModestEasysetupWizardDialog* self)
{
	GError *error = NULL;
	ModestTransportStoreProtocol protocol;
	const gchar* hostname;
	const gchar* username;
	ModestConnectionProtocol protocol_security_incoming;
	int port_num; 
	GList *list_auth_methods;
	ModestEasysetupWizardDialogPrivate *priv;
	
	priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	protocol = easysetup_servertype_combo_box_get_active_servertype (
		 EASYSETUP_SERVERTYPE_COMBO_BOX (priv->combo_incoming_servertype));
	hostname = gtk_entry_get_text(GTK_ENTRY(priv->entry_incomingserver));
	username = gtk_entry_get_text(GTK_ENTRY(priv->entry_user_username));
	protocol_security_incoming = modest_serversecurity_combo_box_get_active_serversecurity (
		MODEST_SERVERSECURITY_COMBO_BOX (priv->combo_incoming_security));
	port_num = get_serverport_incoming(protocol, protocol_security_incoming); 
	list_auth_methods = modest_utils_get_supported_secure_authentication_methods (protocol, hostname, port_num, 
										      username, GTK_WINDOW (self), &error);

	if (list_auth_methods) {
		/* TODO: Select the correct method */
		GList* list = NULL;
		GList* method;
		for (method = list_auth_methods; method != NULL; method = g_list_next(method)) {
			ModestAuthProtocol auth = (ModestAuthProtocol) (GPOINTER_TO_INT(method->data));
			if (modest_protocol_info_auth_is_secure(auth)) {
				list = g_list_append(list, GINT_TO_POINTER(auth));
			}
		}

		g_list_free(list_auth_methods);

		if (list)
			return list;
	}

	if(error == NULL || error->domain != modest_utils_get_supported_secure_authentication_error_quark() ||
			error->code != MODEST_UTILS_GET_SUPPORTED_SECURE_AUTHENTICATION_ERROR_CANCELED)
	{
		show_error (GTK_WIDGET(self), _("mcen_ib_unableto_discover_auth_methods"));
	}

	if(error != NULL)
		g_error_free(error);

	return NULL;
}

static gboolean check_has_supported_auth_methods(ModestEasysetupWizardDialog* self)
{
	GList* methods = check_for_supported_auth_methods(self);
	if (!methods)
	{
		return FALSE;
	}

	g_list_free(methods);
	return TRUE;
}

static ModestAuthProtocol check_first_supported_auth_method(ModestEasysetupWizardDialog* self)
{
	ModestAuthProtocol result = MODEST_PROTOCOL_AUTH_PASSWORD;

	GList* methods = check_for_supported_auth_methods(self);
	if (methods)
	{
		/* Use the first one: */
		result = (ModestAuthProtocol) (GPOINTER_TO_INT(methods->data));
		g_list_free(methods);
	}

	return result;
}

static void
invoke_enable_buttons_vfunc (ModestEasysetupWizardDialog *wizard_dialog)
{
	ModestWizardDialogClass *klass = MODEST_WIZARD_DIALOG_GET_CLASS (wizard_dialog);
	
	/* Call the vfunc, which may be overridden by derived classes: */
	if (klass->enable_buttons) {
		GtkNotebook *notebook = NULL;
		g_object_get (wizard_dialog, "wizard-notebook", &notebook, NULL);
		
		const gint current_page_num = gtk_notebook_get_current_page (notebook);
		if (current_page_num == -1)
			return;
			
		GtkWidget* current_page_widget = gtk_notebook_get_nth_page (notebook, current_page_num);
		(*(klass->enable_buttons))(MODEST_WIZARD_DIALOG (wizard_dialog), current_page_widget);
	}
}

static void
on_caption_entry_changed (GtkEditable *editable, gpointer user_data)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (user_data);
	g_assert(self);
	invoke_enable_buttons_vfunc(self);
}

static void
on_caption_combobox_changed (GtkComboBox *widget, gpointer user_data)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (user_data);
	g_assert(self);
	invoke_enable_buttons_vfunc(self);
}

/** This is a convenience function to create a caption containing a mandatory widget.
 * When the widget is edited, the enable_buttons() vfunc will be called.
 */
static GtkWidget* create_caption_new_with_asterisk(ModestEasysetupWizardDialog *self,
						  GtkSizeGroup *group,
						  const gchar *value,
						  GtkWidget *control,
						  GtkWidget *icon,
						  HildonCaptionStatus flag)
{
	GtkWidget *caption = NULL;
  
	/* Note: Previously, the translated strings already contained the "*",
	 * Comment out this code if they do again.
	 */
	/* Add a * character to indicate mandatory fields,
	 * as specified in our "Email UI Specification": */
	if (flag == HILDON_CAPTION_MANDATORY) {
		gchar* title = g_strdup_printf("%s*", value);
		caption = hildon_caption_new (group, title, control, icon, flag);	
		g_free(title);
	}	
	else
		caption = hildon_caption_new (group, value, control, icon, flag);

	/* Connect to the appropriate changed signal for the widget, 
	 * so we can ask for the prev/next buttons to be enabled/disabled appropriately:
	 */
	if (GTK_IS_ENTRY (control)) {
		g_signal_connect (G_OBJECT (control), "changed",
				  G_CALLBACK (on_caption_entry_changed), self);
		
	}
	else if (GTK_IS_COMBO_BOX (control)) {
		g_signal_connect (G_OBJECT (control), "changed",
				  G_CALLBACK (on_caption_combobox_changed), self);
	}
	 
	return caption;
}
           
static GtkWidget*
create_page_welcome (ModestEasysetupWizardDialog *self)
{
	GtkWidget *box = gtk_vbox_new (FALSE, MODEST_MARGIN_NONE);
	GtkWidget *label = gtk_label_new(_("mcen_ia_emailsetup_intro"));
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	/* So that it is not truncated: */
	gtk_widget_set_size_request (label, 600, -1);
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	gtk_widget_show (GTK_WIDGET (box));
	return GTK_WIDGET (box);
}

static void
on_combo_account_country (GtkComboBox *widget, gpointer user_data)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (user_data);
	g_assert(self);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	
	priv->dirty = TRUE;
	
	/* Fill the providers combo, based on the selected country: */
	if (priv->presets != NULL) {
		gint mcc = easysetup_country_combo_box_get_active_country_mcc (
			MODEST_EASYSETUP_COUNTRY_COMBO_BOX (priv->combo_account_country));
		easysetup_provider_combo_box_fill (
			EASYSETUP_PROVIDER_COMBO_BOX (priv->combo_account_serviceprovider), priv->presets, mcc);
	}
}

static void
on_combo_account_serviceprovider (GtkComboBox *widget, gpointer user_data)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (user_data);
	g_assert(self);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	
	priv->dirty = TRUE;
	
	/* Fill the providers combo, based on the selected country: */
	gchar* provider_id = easysetup_provider_combo_box_get_active_provider_id (
		EASYSETUP_PROVIDER_COMBO_BOX (priv->combo_account_serviceprovider));
	
	gchar* domain_name = NULL;
	if(provider_id)
		domain_name = modest_presets_get_domain (priv->presets, provider_id);
	
	if(!domain_name)
		domain_name = g_strdup (MODEST_EXAMPLE_EMAIL_ADDRESS);
		
	if (priv->entry_user_email)
		gtk_entry_set_text (GTK_ENTRY (priv->entry_user_email), domain_name);
		
	g_free (domain_name);
	
	g_free (provider_id);
}

static void
on_entry_max (ModestValidatingEntry *self, gpointer user_data)
{
	/* ModestEasysetupWizardDialog *dialog = MODEST_EASYSETUP_WIZARD_DIALOG (user_data); */
	show_error (GTK_WIDGET (self), _CS("ckdg_ib_maximum_characters_reached"));
}

static void
on_entry_invalid_character (ModestValidatingEntry *self, const gchar* character, gpointer user_data)
{
	/* ModestEasysetupWizardDialog *dialog = MODEST_EASYSETUP_WIZARD_DIALOG (user_data); */
	
	const gchar *show_char = NULL;
	if (character)
	  show_char = character;
	else {
	  /* TODO: We need a logical ID for this: */
	  show_char = _("whitespace");
	}
	
	/* TODO: Should this show just this one bad character or all the not-allowed characters? */
	gchar *message = g_strdup_printf (_CS("ckdg_ib_illegal_characters_entered"), show_char);
	show_error (GTK_WIDGET (self), message);
}

static gint
get_default_country_code(void)
{
	/* TODO: Default to the current country somehow.
	 * But I don't know how to get the information that is specified in the
	 * "Language and region" control panel. It does not seem be anywhere in gconf. murrayc.
	 *
	 * This is probably not the best choice of gconf key:
	 * This is the	"mcc used in the last pairing", ie. the last connection you made.
	 * set by the osso-operator-wizard package, suggested by Dirk-Jan Binnema.
	 *
	 */
	GError *error = NULL;
	const gchar* key = "/apps/osso/operator-wizard/last_mcc";
	gint mcc_id = modest_conf_get_int(modest_runtime_get_conf (), key, &error);

	if(mcc_id < 0)
		mcc_id = 0;

	if (error) {
		g_warning ("Error getting gconf key %s:\n%s", key, error->message);
		g_error_free (error);
		error = NULL;

		mcc_id = 0;
	}

	/* Note that modest_conf_get_int() seems to return 0 without an error if the key is not there
	 * This might just be a Maemo bug.
	 */
	if (mcc_id == 0)
	{
		/* For now, we default to Finland when there is nothing better: */
		mcc_id = 244;
	}
	return mcc_id;
}

static GtkWidget*
create_page_account_details (ModestEasysetupWizardDialog *self)
{
	GtkWidget *box = gtk_vbox_new (FALSE, MODEST_MARGIN_NONE);
	GtkWidget *label = gtk_label_new(_("mcen_ia_accountdetails"));
	ModestEasysetupWizardDialogPrivate* priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(self);

	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_widget_set_size_request (label, 600, -1);
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (label);

	/* Create a size group to be used by all captions.
	 * Note that HildonCaption does not create a default size group if we do not specify one.
	 * We use GTK_SIZE_GROUP_HORIZONTAL, so that the widths are the same. */
	GtkSizeGroup* sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	/* The country widgets: */
	priv->combo_account_country = GTK_WIDGET (easysetup_country_combo_box_new ());
	GtkWidget *caption = create_caption_new_with_asterisk (self, sizegroup, _("mcen_fi_country"), 
							      priv->combo_account_country, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (priv->combo_account_country);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	/* connect to country combo's changed signal, so we can fill the provider combo: */
	g_signal_connect (G_OBJECT (priv->combo_account_country), "changed",
			  G_CALLBACK (on_combo_account_country), self);
            
	GtkWidget *separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (box), separator, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (separator);
            
	/* The service provider widgets: */	
	priv->combo_account_serviceprovider = GTK_WIDGET (easysetup_provider_combo_box_new ());
	gtk_widget_set_size_request (priv->combo_account_serviceprovider, 320, -1);
	
	caption = create_caption_new_with_asterisk (self, sizegroup, _("mcen_fi_serviceprovider"), 
						   priv->combo_account_serviceprovider, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (priv->combo_account_serviceprovider);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	/* connect to providers combo's changed signal, so we can fill the email address: */
	g_signal_connect (G_OBJECT (priv->combo_account_serviceprovider), "changed",
			  G_CALLBACK (on_combo_account_serviceprovider), self);
	
	/* The description widgets: */	
	priv->entry_account_title = GTK_WIDGET (modest_validating_entry_new ());
	g_signal_connect(G_OBJECT(priv->entry_account_title), "changed",
			 G_CALLBACK(on_easysetup_changed), self);
	/* Do use auto-capitalization: */
	hildon_gtk_entry_set_input_mode (GTK_ENTRY (priv->entry_account_title), 
					 HILDON_GTK_INPUT_MODE_FULL | HILDON_GTK_INPUT_MODE_AUTOCAP);
	
	/* Set a default account title, choosing one that does not already exist: */
	/* Note that this is irrelevant to the non-user visible name, which we will create later. */
	gchar* default_account_name_start = g_strdup (_("mcen_ia_emailsetup_defaultname"));
	gchar* default_account_name = modest_account_mgr_get_unused_account_display_name (
		priv->account_manager, default_account_name_start);
	g_free (default_account_name_start);
	default_account_name_start = NULL;
	
	gtk_entry_set_text( GTK_ENTRY (priv->entry_account_title), default_account_name);
	g_free (default_account_name);
	default_account_name = NULL;

	caption = create_caption_new_with_asterisk (self, sizegroup, _("mcen_fi_account_title"), 
						   priv->entry_account_title, NULL, HILDON_CAPTION_MANDATORY);
	gtk_widget_show (priv->entry_account_title);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	/* Prevent the use of some characters in the account title, 
	 * as required by our UI specification: */
	GList *list_prevent = NULL;
	list_prevent = g_list_append (list_prevent, "\\");
	list_prevent = g_list_append (list_prevent, "/");
	list_prevent = g_list_append (list_prevent, ":");
	list_prevent = g_list_append (list_prevent, "*");
	list_prevent = g_list_append (list_prevent, "?");
	list_prevent = g_list_append (list_prevent, "\""); /* The UI spec mentions “, but maybe means ", maybe both. */
	list_prevent = g_list_append (list_prevent, "“");
	list_prevent = g_list_append (list_prevent, "<"); 
	list_prevent = g_list_append (list_prevent, ">"); 
	list_prevent = g_list_append (list_prevent, "|");
	list_prevent = g_list_append (list_prevent, "^"); 	
	modest_validating_entry_set_unallowed_characters (
	 	MODEST_VALIDATING_ENTRY (priv->entry_account_title), list_prevent);
	g_list_free (list_prevent);
	list_prevent = NULL;
	modest_validating_entry_set_func(MODEST_VALIDATING_ENTRY(priv->entry_account_title),
																	 on_entry_invalid_character, self);
	
	/* Set max length as in the UI spec:
	 * The UI spec seems to want us to show a dialog if we hit the maximum. */
	gtk_entry_set_max_length (GTK_ENTRY (priv->entry_account_title), 64);
	modest_validating_entry_set_max_func (MODEST_VALIDATING_ENTRY (priv->entry_account_title), 
					      on_entry_max, self);
	
	gtk_widget_show (GTK_WIDGET (box));
	
	return GTK_WIDGET (box);
}

static GtkWidget*
create_page_user_details (ModestEasysetupWizardDialog *self)
{
	GtkSizeGroup* sizegroup;
	GtkWidget *box;
	ModestEasysetupWizardDialogPrivate *priv;

	priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(self);
	
	/* Create a size group to be used by all captions.
	 * Note that HildonCaption does not create a default size group if we do not specify one.
	 * We use GTK_SIZE_GROUP_HORIZONTAL, so that the widths are the same. */
	box = gtk_vbox_new (FALSE, MODEST_MARGIN_NONE);
	sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	 
	/* The name widgets: (use auto cap) */
	priv->entry_user_name = GTK_WIDGET (modest_validating_entry_new ());
	hildon_gtk_entry_set_input_mode (GTK_ENTRY (priv->entry_user_name), 
					 HILDON_GTK_INPUT_MODE_FULL | HILDON_GTK_INPUT_MODE_AUTOCAP);
	
	/* Set max length as in the UI spec:
	 * The UI spec seems to want us to show a dialog if we hit the maximum. */
	gtk_entry_set_max_length (GTK_ENTRY (priv->entry_user_name), 64);
	modest_validating_entry_set_max_func (MODEST_VALIDATING_ENTRY (priv->entry_user_name), 
					      on_entry_max, self);
	GtkWidget *caption = create_caption_new_with_asterisk (self, sizegroup, 
							      _("mcen_li_emailsetup_name"), priv->entry_user_name, NULL, HILDON_CAPTION_OPTIONAL);
	g_signal_connect(G_OBJECT(priv->entry_user_name), "changed", 
									 G_CALLBACK(on_easysetup_changed), self);
	gtk_widget_show (priv->entry_user_name);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	/* Prevent the use of some characters in the name, 
	 * as required by our UI specification: */
	GList *list_prevent = NULL;
	list_prevent = g_list_append (list_prevent, "<");
	list_prevent = g_list_append (list_prevent, ">");
	modest_validating_entry_set_unallowed_characters (
	 	MODEST_VALIDATING_ENTRY (priv->entry_user_name), list_prevent);
	modest_validating_entry_set_func(MODEST_VALIDATING_ENTRY(priv->entry_user_name),
		on_entry_invalid_character, self);
	g_list_free (list_prevent);
	
	/* The username widgets: */	
	priv->entry_user_username = GTK_WIDGET (modest_validating_entry_new ());
	/* Auto-capitalization is the default, so let's turn it off: */
	hildon_gtk_entry_set_input_mode (GTK_ENTRY (priv->entry_user_username), HILDON_GTK_INPUT_MODE_FULL);
	caption = create_caption_new_with_asterisk (self, sizegroup, _("mail_fi_username"), 
						   priv->entry_user_username, NULL, HILDON_CAPTION_MANDATORY);
	gtk_widget_show (priv->entry_user_username);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	g_signal_connect(G_OBJECT(priv->entry_user_username), "changed", 
									 G_CALLBACK(on_easysetup_changed), self);
	gtk_widget_show (caption);
	
	/* Prevent the use of some characters in the username, 
	 * as required by our UI specification: */
	modest_validating_entry_set_unallowed_characters_whitespace (
	 	MODEST_VALIDATING_ENTRY (priv->entry_user_username));
	modest_validating_entry_set_func(MODEST_VALIDATING_ENTRY(priv->entry_user_username),
		on_entry_invalid_character, self);
	
	/* Set max length as in the UI spec:
	 * The UI spec seems to want us to show a dialog if we hit the maximum. */
	gtk_entry_set_max_length (GTK_ENTRY (priv->entry_user_username), 64);
	modest_validating_entry_set_max_func (MODEST_VALIDATING_ENTRY (priv->entry_user_username), 
					      on_entry_max, self);
	
	/* The password widgets: */	
	priv->entry_user_password = gtk_entry_new ();
	/* Auto-capitalization is the default, so let's turn it off: */
	hildon_gtk_entry_set_input_mode (GTK_ENTRY (priv->entry_user_password), 
					 HILDON_GTK_INPUT_MODE_FULL | HILDON_GTK_INPUT_MODE_INVISIBLE);
	gtk_entry_set_visibility (GTK_ENTRY (priv->entry_user_password), FALSE);
	/* gtk_entry_set_invisible_char (GTK_ENTRY (priv->entry_user_password), '*'); */
	caption = create_caption_new_with_asterisk (self, sizegroup, 
						   _("mail_fi_password"), priv->entry_user_password, NULL, HILDON_CAPTION_OPTIONAL);
	g_signal_connect(G_OBJECT(priv->entry_user_password), "changed", 
									 G_CALLBACK(on_easysetup_changed), self);
	gtk_widget_show (priv->entry_user_password);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	/* The email address widgets: */	
	priv->entry_user_email = GTK_WIDGET (modest_validating_entry_new ());
	/* Auto-capitalization is the default, so let's turn it off: */
	hildon_gtk_entry_set_input_mode (GTK_ENTRY (priv->entry_user_email), HILDON_GTK_INPUT_MODE_FULL);
	caption = create_caption_new_with_asterisk (self, sizegroup, 
						   _("mcen_li_emailsetup_email_address"), priv->entry_user_email, NULL, HILDON_CAPTION_MANDATORY);
	gtk_entry_set_text (GTK_ENTRY (priv->entry_user_email), MODEST_EXAMPLE_EMAIL_ADDRESS); /* Default text. */
	gtk_widget_show (priv->entry_user_email);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	g_signal_connect(G_OBJECT(priv->entry_user_email), "changed", 
									 G_CALLBACK(on_easysetup_changed), self);
	gtk_widget_show (caption);
	
	/* Set max length as in the UI spec:
	 * The UI spec seems to want us to show a dialog if we hit the maximum. */
	gtk_entry_set_max_length (GTK_ENTRY (priv->entry_user_email), 64);
	modest_validating_entry_set_max_func (MODEST_VALIDATING_ENTRY (priv->entry_user_email), 
					      on_entry_max, self);
	
	
	gtk_widget_show (GTK_WIDGET (box));
	
	return GTK_WIDGET (box);
}

static GtkWidget* create_page_complete_easysetup (ModestEasysetupWizardDialog *self)
{
	GtkWidget *box = gtk_vbox_new (FALSE, MODEST_MARGIN_NONE);
	
	GtkWidget *label = gtk_label_new(_("mcen_ia_emailsetup_setup_complete"));
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_widget_set_size_request (label, 600, -1);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	/* The documentation for gtk_label_set_line_wrap() says that we must 
	 * call gtk_widget_set_size_request() with a hard-coded width, 
	 * though I wonder why gtk_label_set_max_width_chars() isn't enough. */
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	
	label = gtk_label_new (_("mcen_ia_easysetup_complete"));
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_widget_set_size_request (label, 600, -1);
	
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	
	gtk_widget_show (GTK_WIDGET (box));
	return GTK_WIDGET (box);
}

/** Change the caption title for the incoming server, 
 * as specified in the UI spec:
 */
static void 
update_incoming_server_title (ModestEasysetupWizardDialog *self)
{
	ModestEasysetupWizardDialogPrivate* priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(self);
	const ModestTransportStoreProtocol protocol = easysetup_servertype_combo_box_get_active_servertype (
		EASYSETUP_SERVERTYPE_COMBO_BOX (priv->combo_incoming_servertype));
	const gchar* type = 
		(protocol == MODEST_PROTOCOL_STORE_POP ? 
		 _("mail_fi_emailtype_pop3") : 
		 _("mail_fi_emailtype_imap") );
			
		
	/* Note that this produces a compiler warning, 
	 * because the compiler does not know that the translated string will have a %s in it.
	 * I do not see a way to avoid the warning while still using these Logical IDs. murrayc. */
	gchar* incomingserver_title = g_strdup_printf(_("mcen_li_emailsetup_servertype"), type);
	g_object_set (G_OBJECT (priv->caption_incoming), "label", incomingserver_title, NULL);
	g_free(incomingserver_title);
}

/** Change the caption title for the incoming server, 
 * as specified in the UI spec:
 */
static void 
update_incoming_server_security_choices (ModestEasysetupWizardDialog *self)
{
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(self);
	const ModestTransportStoreProtocol protocol = easysetup_servertype_combo_box_get_active_servertype (
		EASYSETUP_SERVERTYPE_COMBO_BOX (priv->combo_incoming_servertype));
	
	/* Fill the combo with appropriately titled choices for POP or IMAP. */
	/* The choices are the same, but the titles are different, as in the UI spec. */
	modest_serversecurity_combo_box_fill (
		MODEST_SERVERSECURITY_COMBO_BOX (priv->combo_incoming_security), protocol);
}

static void 
on_combo_servertype_changed(GtkComboBox *combobox, gpointer user_data)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (user_data);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(self);
	
	priv->dirty = TRUE;
	
	update_incoming_server_title (self);
	update_incoming_server_security_choices (self);

	set_default_custom_servernames (self);
}

static void 
on_entry_incoming_servername_changed(GtkEntry *entry, gpointer user_data)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (user_data);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	priv->dirty = TRUE;
	priv->server_changes |= MODEST_EASYSETUP_WIZARD_DIALOG_INCOMING_CHANGED;
}

static GtkWidget* 
create_page_custom_incoming (ModestEasysetupWizardDialog *self)
{
	ModestEasysetupWizardDialogPrivate* priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(self);
	GtkWidget *box = gtk_vbox_new (FALSE, MODEST_MARGIN_NONE);
	GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
					GTK_POLICY_NEVER,
					GTK_POLICY_AUTOMATIC);

	/* Show note that account type cannot be changed in future: */
	GtkWidget *label = gtk_label_new (_("mcen_ia_emailsetup_account_type"));
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_widget_set_size_request (label, 600, -1);
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	
	/* Create a size group to be used by all captions.
	 * Note that HildonCaption does not create a default size group if we do not specify one.
	 * We use GTK_SIZE_GROUP_HORIZONTAL, so that the widths are the same. */
	GtkSizeGroup *sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	 
	/* The incoming server widgets: */
	priv->combo_incoming_servertype = GTK_WIDGET (easysetup_servertype_combo_box_new ());
	easysetup_servertype_combo_box_set_active_servertype (
		EASYSETUP_SERVERTYPE_COMBO_BOX (priv->combo_incoming_servertype), MODEST_PROTOCOL_STORE_POP);
	GtkWidget *caption = create_caption_new_with_asterisk (self, sizegroup, 
							      _("mcen_li_emailsetup_type"), priv->combo_incoming_servertype, NULL, HILDON_CAPTION_MANDATORY);
	gtk_widget_show (priv->combo_incoming_servertype);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	priv->entry_incomingserver = gtk_entry_new ();
	g_signal_connect(G_OBJECT(priv->entry_incomingserver), "changed", G_CALLBACK(on_easysetup_changed), self);
	/* Auto-capitalization is the default, so let's turn it off: */
	hildon_gtk_entry_set_input_mode (GTK_ENTRY (priv->entry_incomingserver), HILDON_GTK_INPUT_MODE_FULL);
	set_default_custom_servernames (self);

	/* The caption title will be updated in update_incoming_server_title().
	 * so this default text will never be seen: */
	/* (Note: Changing the title seems pointless. murrayc) */
	priv->caption_incoming = create_caption_new_with_asterisk (self, sizegroup, 
								  "Incoming Server", priv->entry_incomingserver, NULL, HILDON_CAPTION_MANDATORY);
	update_incoming_server_title (self);
	gtk_widget_show (priv->entry_incomingserver);
	gtk_box_pack_start (GTK_BOX (box), priv->caption_incoming, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (priv->caption_incoming);
	
	/* Change the caption title when the servertype changes, 
	 * as in the UI spec: */
	g_signal_connect (G_OBJECT (priv->combo_incoming_servertype), "changed",
			  G_CALLBACK (on_combo_servertype_changed), self);

	/* Remember when the servername was changed manually: */
	g_signal_connect (G_OBJECT (priv->entry_incomingserver), "changed",
	                  G_CALLBACK (on_entry_incoming_servername_changed), self);

	/* The secure connection widgets: */	
	priv->combo_incoming_security = GTK_WIDGET (modest_serversecurity_combo_box_new ());
	update_incoming_server_security_choices (self);
	modest_serversecurity_combo_box_set_active_serversecurity (
		MODEST_SERVERSECURITY_COMBO_BOX (priv->combo_incoming_security), MODEST_PROTOCOL_CONNECTION_NORMAL);
	caption = hildon_caption_new (sizegroup, _("mcen_li_emailsetup_secure_connection"), 
				      priv->combo_incoming_security, NULL, HILDON_CAPTION_OPTIONAL);
	g_signal_connect (G_OBJECT (priv->combo_incoming_security), "changed",
	                  G_CALLBACK (on_incoming_security_changed), self);
	gtk_widget_show (priv->combo_incoming_security);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	priv->checkbox_incoming_auth = gtk_check_button_new ();
	g_signal_connect (G_OBJECT (priv->checkbox_incoming_auth), "toggled",
	                  G_CALLBACK (on_easysetup_changed), self);
	caption = hildon_caption_new (sizegroup, _("mcen_li_emailsetup_secure_authentication"), 
				      priv->checkbox_incoming_auth, NULL, HILDON_CAPTION_OPTIONAL);
	
	gtk_widget_show (priv->checkbox_incoming_auth);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	gtk_widget_show (GTK_WIDGET (box));

	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), box);
	gtk_container_set_focus_vadjustment (GTK_CONTAINER (box),
					     gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (scrolled_window)));
	gtk_widget_show (scrolled_window);
	
	return GTK_WIDGET (scrolled_window);
}

static void
on_toggle_button_changed (GtkToggleButton *togglebutton, gpointer user_data)
{
	GtkWidget *widget = GTK_WIDGET (user_data);
	
	/* Enable the widget only if the toggle button is active: */
	const gboolean enable = gtk_toggle_button_get_active (togglebutton);
	gtk_widget_set_sensitive (widget, enable);
}

/* Make the sensitivity of a widget depend on a toggle button.
 */
static void
enable_widget_for_togglebutton (GtkWidget *widget, GtkToggleButton* button)
{
	g_signal_connect (G_OBJECT (button), "toggled",
			  G_CALLBACK (on_toggle_button_changed), widget);
	
	/* Set the starting sensitivity: */
	on_toggle_button_changed (button, widget);
}

static void
on_button_outgoing_smtp_servers (GtkButton *button, gpointer user_data)
{
	ModestEasysetupWizardDialog * self = MODEST_EASYSETUP_WIZARD_DIALOG (user_data);
	ModestEasysetupWizardDialogPrivate* priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(self);
	
	/* We set dirty here because setting it depending on the connection specific dialog
	seems overkill */
	priv->dirty = TRUE;
	
	/* Create the window, if necessary: */
	if (!(priv->specific_window)) {
		priv->specific_window = GTK_WIDGET (modest_connection_specific_smtp_window_new ());
		modest_connection_specific_smtp_window_fill_with_connections (
			MODEST_CONNECTION_SPECIFIC_SMTP_WINDOW (priv->specific_window), priv->account_manager);
	}

	/* Show the window: */
	gtk_window_set_transient_for (GTK_WINDOW (priv->specific_window), GTK_WINDOW (self));
	gtk_window_set_modal (GTK_WINDOW (priv->specific_window), TRUE);
	gtk_widget_show (priv->specific_window);
}

static void 
on_entry_outgoing_servername_changed (GtkEntry *entry, gpointer user_data)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (user_data);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	priv->server_changes |= MODEST_EASYSETUP_WIZARD_DIALOG_OUTGOING_CHANGED;
}

static GtkWidget* 
create_page_custom_outgoing (ModestEasysetupWizardDialog *self)
{
	ModestEasysetupWizardDialogPrivate *priv;
	GtkWidget *box = gtk_vbox_new (FALSE, MODEST_MARGIN_NONE);
	
	/* Create a size group to be used by all captions.
	 * Note that HildonCaption does not create a default size group if we do not specify one.
	 * We use GTK_SIZE_GROUP_HORIZONTAL, so that the widths are the same. */
	GtkSizeGroup *sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	 
	/* The outgoing server widgets: */
	priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	priv->entry_outgoingserver = gtk_entry_new ();
	g_signal_connect (G_OBJECT (priv->entry_outgoingserver), "changed",
                  G_CALLBACK (on_easysetup_changed), self);
	/* Auto-capitalization is the default, so let's turn it off: */
	hildon_gtk_entry_set_input_mode (GTK_ENTRY (priv->entry_outgoingserver), HILDON_GTK_INPUT_MODE_FULL);
	GtkWidget *caption = create_caption_new_with_asterisk (self, sizegroup, 
							      _("mcen_li_emailsetup_smtp"), priv->entry_outgoingserver, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (priv->entry_outgoingserver);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	set_default_custom_servernames (self);
	
	/* The secure connection widgets: */	
	priv->combo_outgoing_security = GTK_WIDGET (modest_serversecurity_combo_box_new ());
	g_signal_connect (G_OBJECT (priv->combo_outgoing_security), "changed",
	                  G_CALLBACK (on_easysetup_changed), self);
	modest_serversecurity_combo_box_fill (
		MODEST_SERVERSECURITY_COMBO_BOX (priv->combo_outgoing_security), MODEST_PROTOCOL_TRANSPORT_SMTP);
	modest_serversecurity_combo_box_set_active_serversecurity (
		MODEST_SERVERSECURITY_COMBO_BOX (priv->combo_outgoing_security), MODEST_PROTOCOL_CONNECTION_NORMAL);
	caption = hildon_caption_new (sizegroup, _("mcen_li_emailsetup_secure_connection"), 
				      priv->combo_outgoing_security, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (priv->combo_outgoing_security);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	/* The secure authentication widgets: */
	priv->combo_outgoing_auth = GTK_WIDGET (modest_secureauth_combo_box_new ());
			g_signal_connect (G_OBJECT (priv->combo_outgoing_auth), "changed",
	                  G_CALLBACK (on_easysetup_changed), self);
	caption = hildon_caption_new (sizegroup, _("mcen_li_emailsetup_secure_authentication"), 
				      priv->combo_outgoing_auth, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (priv->combo_outgoing_auth);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	GtkWidget *separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (box), separator, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (separator);
	
	/* connection-specific checkbox: */
	priv->checkbox_outgoing_smtp_specific = gtk_check_button_new ();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbox_outgoing_smtp_specific), 
				      FALSE);
	g_signal_connect (G_OBJECT (priv->checkbox_outgoing_smtp_specific), "toggled",
                  G_CALLBACK (on_easysetup_changed), self);

	caption = hildon_caption_new (sizegroup, _("mcen_fi_advsetup_connection_smtp"), 
				      priv->checkbox_outgoing_smtp_specific, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (priv->checkbox_outgoing_smtp_specific);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	/* Connection-specific SMTP-Severs Edit button: */
	priv->button_outgoing_smtp_servers = gtk_button_new_with_label (_("mcen_bd_edit"));
	caption = hildon_caption_new (sizegroup, _("mcen_fi_advsetup_optional_smtp"), 
				      priv->button_outgoing_smtp_servers, NULL, HILDON_CAPTION_OPTIONAL);
	hildon_caption_set_child_expand (HILDON_CAPTION (caption), FALSE);
	gtk_widget_show (priv->button_outgoing_smtp_servers);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	/* Only enable the button when the checkbox is checked: */
	enable_widget_for_togglebutton (priv->button_outgoing_smtp_servers, 
					GTK_TOGGLE_BUTTON (priv->checkbox_outgoing_smtp_specific));
		
	g_signal_connect (G_OBJECT (priv->button_outgoing_smtp_servers), "clicked",
			  G_CALLBACK (on_button_outgoing_smtp_servers), self);

	g_signal_connect (G_OBJECT (priv->entry_outgoingserver), "changed",
	                  G_CALLBACK (on_entry_outgoing_servername_changed), self);
	
	
	gtk_widget_show (GTK_WIDGET (box));
	
	return GTK_WIDGET (box);
}

static gboolean
show_advanced_edit(gpointer user_data)
{
	ModestEasysetupWizardDialog * self = MODEST_EASYSETUP_WIZARD_DIALOG (user_data);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	gint response;
	
	/* Show the Account Settings window: */
	ModestAccountSettingsDialog *dialog = modest_account_settings_dialog_new ();
	if (priv->pending_load_settings) {
		save_to_settings (self);
		priv->pending_load_settings = FALSE;
	}
	modest_account_settings_dialog_set_account (dialog, priv->settings);
	
	modest_window_mgr_set_modal (modest_runtime_get_window_mgr (), GTK_WINDOW (dialog));
	
	response = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (GTK_WIDGET (dialog));
	
	return FALSE; /* Do not call this timeout callback again. */
}

static void
on_button_edit_advanced_settings (GtkButton *button, gpointer user_data)
{
	ModestEasysetupWizardDialog * self = MODEST_EASYSETUP_WIZARD_DIALOG (user_data);
	
	/* Show the Account Settings window: */
	show_advanced_edit(self);
}
static GtkWidget* create_page_complete_custom (ModestEasysetupWizardDialog *self)
{
	GtkWidget *box = gtk_vbox_new (FALSE, MODEST_MARGIN_NONE);
	GtkWidget *label = gtk_label_new(_("mcen_ia_emailsetup_setup_complete"));
	GtkWidget *button_edit = gtk_button_new_with_label (_("mcen_bd_edit"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_widget_set_size_request (label, 600, -1);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	
	label = gtk_label_new (_("mcen_ia_customsetup_complete"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_widget_set_size_request (label, 600, -1);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	
	GtkWidget *caption = hildon_caption_new (NULL, _("mcen_fi_advanced_settings"), 
						 button_edit, NULL, HILDON_CAPTION_OPTIONAL);
	hildon_caption_set_child_expand (HILDON_CAPTION (caption), FALSE);
	gtk_widget_show (button_edit);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, MODEST_MARGIN_HALF);
	gtk_widget_show (caption);
	
	g_signal_connect (G_OBJECT (button_edit), "clicked", 
			  G_CALLBACK (on_button_edit_advanced_settings), self);
	
	gtk_widget_show (GTK_WIDGET (box));
	return GTK_WIDGET (box);
}


/*
 */
static void 
on_response (ModestWizardDialog *wizard_dialog,
	     gint response_id,
	     gpointer user_data)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (wizard_dialog);

	invoke_enable_buttons_vfunc (self);
}

static void 
on_response_before (ModestWizardDialog *wizard_dialog,
                    gint response_id,
                    gpointer user_data)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (wizard_dialog);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(wizard_dialog);
	if (response_id == GTK_RESPONSE_CANCEL) {
		/* This is mostly copied from
		 * src/maemo/modest-account-settings-dialog.c */
		if (priv->dirty) {
			GtkDialog *dialog = GTK_DIALOG (hildon_note_new_confirmation (GTK_WINDOW (self), 
				_("imum_nc_wizard_confirm_lose_changes")));
			/* TODO: These button names will be ambiguous, and not
			 * specified in the UI specification. */

			const gint dialog_response = gtk_dialog_run (dialog);
			gtk_widget_destroy (GTK_WIDGET (dialog));

			if (dialog_response != GTK_RESPONSE_OK) {
				/* Don't let the dialog close */
				g_signal_stop_emission_by_name (wizard_dialog, "response");
			}
		}
	}
}

typedef struct IdleData {
	ModestEasysetupWizardDialog *dialog;
	ModestPresets *presets;
} IdleData;

static gboolean
presets_idle (gpointer userdata)
{
	IdleData *idle_data = (IdleData *) userdata;
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (idle_data->dialog);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);

	g_assert (idle_data->presets);

	gdk_threads_enter ();

	priv->presets = idle_data->presets;

	if (MODEST_EASYSETUP_IS_COUNTRY_COMBO_BOX (priv->combo_account_country)) {
		gint mcc = get_default_country_code();
		/* Fill the combo in an idle call, as it takes a lot of time */
		easysetup_country_combo_box_load_data(
			MODEST_EASYSETUP_COUNTRY_COMBO_BOX (priv->combo_account_country));
		easysetup_country_combo_box_set_active_country_mcc (
			MODEST_EASYSETUP_COUNTRY_COMBO_BOX (priv->combo_account_country), mcc);
		easysetup_provider_combo_box_fill (
			EASYSETUP_PROVIDER_COMBO_BOX (priv->combo_account_serviceprovider),
			priv->presets, mcc);
	}

	priv->dirty = FALSE;

	g_object_unref (idle_data->dialog);
	g_free (idle_data);

	gdk_threads_leave ();

	return FALSE;
}

static gpointer
presets_loader (gpointer userdata)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (userdata);
	ModestPresets *presets = NULL;
	IdleData *idle_data;

	const gchar* path  = NULL;
	const gchar* path1 = MODEST_PROVIDER_DATA_FILE;
	const gchar* path2 = MODEST_MAEMO_PROVIDER_DATA_FILE;
	
	if (access(path1, R_OK) == 0) 
		path = path1;
	else if (access(path2, R_OK) == 0)
		path = path2;
	else {
		g_warning ("%s: neither '%s' nor '%s' is a readable provider data file",
			   __FUNCTION__, path1, path2);
		return NULL;
	}

	presets = modest_presets_new (path);
	if (!presets) {
		g_warning ("%s: failed to parse '%s'", __FUNCTION__, path);
		return NULL;
	}
	
	idle_data = g_new0 (IdleData, 1);
	idle_data->dialog = self;
	idle_data->presets = presets;
	
	g_idle_add (presets_idle, idle_data);	

	return NULL;
}

static void
modest_easysetup_wizard_dialog_init (ModestEasysetupWizardDialog *self)
{
	gtk_container_set_border_width (GTK_CONTAINER (self), MODEST_MARGIN_HALF);
	
	/* Create the notebook to be used by the ModestWizardDialog base class:
	 * Each page of the notebook will be a page of the wizard: */
	GtkNotebook *notebook = GTK_NOTEBOOK (gtk_notebook_new());
	
	/* Set the notebook used by the ModestWizardDialog base class: */
	g_object_set (G_OBJECT(self), "wizard-notebook", notebook, NULL);
    
	/* Set the wizard title:
	 * The actual window title will be a combination of this and the page's tab label title. */
	g_object_set (G_OBJECT(self), "wizard-name", _("mcen_ti_emailsetup"), NULL);

	/* Read in the information about known service providers: */
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	
	/* The server fields did not have been manually changed yet */
	priv->server_changes = 0;
	priv->pending_load_settings = TRUE;

	/* Get the account manager object, 
	 * so we can check for existing accounts,
	 * and create new accounts: */
	priv->account_manager = modest_runtime_get_account_mgr ();
	g_object_ref (priv->account_manager);
	
	/* Initialize fields */
	priv->page_welcome = create_page_welcome (self);
	priv->page_account_details = create_page_account_details (self);
	priv->page_user_details = create_page_user_details (self);
	priv->page_complete_easysetup = NULL;       
	priv->page_custom_incoming = NULL;
	priv->combo_incoming_servertype = NULL;
	priv->caption_incoming = NULL;
	priv->entry_incomingserver = NULL;
	priv->combo_incoming_security = NULL;
	priv->checkbox_incoming_auth = NULL;
	priv->page_custom_outgoing = NULL;
	priv->entry_outgoingserver = NULL;
	priv->combo_outgoing_security = NULL;
	priv->combo_outgoing_auth = NULL;
	priv->checkbox_outgoing_smtp_specific = NULL;
	priv->button_outgoing_smtp_servers = NULL;
	priv->page_complete_customsetup = NULL;
	priv->specific_window = NULL;

	/* Add the common pages: */
	gtk_notebook_append_page (notebook, priv->page_welcome, 
				  gtk_label_new (_("mcen_ti_emailsetup_welcome")));
	gtk_container_child_set (GTK_CONTAINER (notebook), 
				 priv->page_welcome,
				 "tab-expand", TRUE,
				 "tab-fill", TRUE,
				 NULL);

	gtk_notebook_append_page (notebook, priv->page_account_details, 
				  gtk_label_new (_("mcen_ti_accountdetails")));
	gtk_container_child_set (GTK_CONTAINER (notebook), 
				 priv->page_account_details,
				 "tab-expand", TRUE,
				 "tab-fill", TRUE,
				 NULL);

	gtk_notebook_append_page (notebook, priv->page_user_details, 
				  gtk_label_new (_("mcen_ti_emailsetup_userdetails")));
	gtk_container_child_set (GTK_CONTAINER (notebook), 
				 priv->page_user_details,
				 "tab-expand", TRUE,
				 "tab-fill", TRUE,
				 NULL);
		
	/* Create and add the easysetup-specific pages,
	 * because we need _some_ final page to enable the Next and Finish buttons: */
	create_subsequent_easysetup_pages (self);

	/* Connect to the dialog's response signal so we can enable/disable buttons 
	 * for the newly-selected page, because the prev/next buttons cause response to be emitted.
	 * Note that we use g_signal_connect_after() instead of g_signal_connect()
	 * so that we can be enable/disable after ModestWizardDialog has done its own 
	 * enabling/disabling of buttons.
	 * 
	 * HOWEVER, this doesn't work because ModestWizardDialog's response signal handler 
	 * does g_signal_stop_emission_by_name(), stopping our signal handler from running.
	 * 
	 * It's not enough to connect to the notebook's switch-page signal, because 
	 * ModestWizardDialog's "response" signal handler enables the buttons itself, 
	 * _after_ switching the page (understandably).
	 * (Note that if we had, if we used g_signal_connect() instead of g_signal_connect_after()
	 * then gtk_notebook_get_current_page() would return an incorrect value.)
	 */
	g_signal_connect_after (G_OBJECT (self), "response",
				G_CALLBACK (on_response), self);

	/* This is to show a confirmation dialog when the user hits cancel */
	g_signal_connect (G_OBJECT (self), "response",
	                  G_CALLBACK (on_response_before), self);

	g_signal_connect (G_OBJECT (self), "delete-event",
			  G_CALLBACK (on_delete_event), self);

	/* Reset dirty, because there was no user input until now */
	priv->dirty = FALSE;
	
	/* When this window is shown, hibernation should not be possible, 
	 * because there is no sensible way to save the state: */
	modest_window_mgr_prevent_hibernation_while_window_is_shown (
		modest_runtime_get_window_mgr (), GTK_WINDOW (self)); 

	/* Load provider presets */
	g_object_ref (self);
	g_thread_create (presets_loader, self, FALSE, NULL);

	hildon_help_dialog_help_enable (GTK_DIALOG(self), "applications_email_wizardwelcome",
					modest_maemo_utils_get_osso_context());	

	priv->settings = modest_account_settings_new ();
}

ModestEasysetupWizardDialog*
modest_easysetup_wizard_dialog_new (void)
{	
	
	return g_object_new (MODEST_TYPE_EASYSETUP_WIZARD_DIALOG, NULL);
}

static void create_subsequent_customsetup_pages (ModestEasysetupWizardDialog *self)
{
	ModestEasysetupWizardDialogPrivate *priv;
	GtkNotebook *notebook = NULL;

	g_object_get (self, "wizard-notebook", &notebook, NULL);
	g_assert(notebook);

	priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	
	/* Create the custom pages: */
	if(!(priv->page_custom_incoming)) {
		priv->page_custom_incoming = create_page_custom_incoming (self);
	}
		
	if(!(priv->page_custom_outgoing)) {
		priv->page_custom_outgoing = create_page_custom_outgoing (self);
	}
	
	if(!(priv->page_complete_customsetup)) {
		priv->page_complete_customsetup = create_page_complete_custom (self);
	}
	
	if (!gtk_widget_get_parent (GTK_WIDGET (priv->page_custom_incoming)))
		gtk_notebook_append_page (notebook, priv->page_custom_incoming,
					  gtk_label_new (_("mcen_ti_emailsetup_incomingdetails")));
	
	if (!gtk_widget_get_parent (GTK_WIDGET (priv->page_custom_outgoing)))		
		gtk_notebook_append_page (notebook, priv->page_custom_outgoing,
					  gtk_label_new (_("mcen_ti_emailsetup_outgoingdetails")));
		
	if (!gtk_widget_get_parent (GTK_WIDGET (priv->page_complete_customsetup))) {
		gtk_notebook_append_page (notebook, priv->page_complete_customsetup,
					  gtk_label_new (_("mcen_ti_emailsetup_complete")));
		gtk_container_child_set (GTK_CONTAINER (notebook), 
					 priv->page_complete_customsetup,
					 "tab-expand", TRUE,
					 "tab-fill", TRUE,
					 NULL);
	}
			
	/* This is unnecessary with GTK+ 2.10: */
	modest_wizard_dialog_force_title_update (MODEST_WIZARD_DIALOG(self));
}
	
static void 
create_subsequent_easysetup_pages (ModestEasysetupWizardDialog *self)
{
	ModestEasysetupWizardDialogPrivate *priv;
	GtkNotebook *notebook = NULL;

	g_object_get (self, "wizard-notebook", &notebook, NULL);
	g_assert(notebook);
	
	/* Create the easysetup-specific pages: */
	priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	if(!priv->page_complete_easysetup)
		priv->page_complete_easysetup = create_page_complete_easysetup (self);

	if (!gtk_widget_get_parent (GTK_WIDGET (priv->page_complete_easysetup))) {
		gtk_notebook_append_page (notebook, priv->page_complete_easysetup, 
					  gtk_label_new (_("mcen_ti_emailsetup_complete")));
		gtk_container_child_set (GTK_CONTAINER (notebook),
					 priv->page_complete_easysetup,
					 "tab-expand", TRUE,
					 "tab-fill", TRUE,
					 NULL);
	}
			
	/* This is unnecessary with GTK+ 2.10: */
	modest_wizard_dialog_force_title_update (MODEST_WIZARD_DIALOG(self));
}
/* After the user details page,
 * the following pages depend on whether "Other" was chosen 
 * in the provider combobox on the account page
 */
static void create_subsequent_pages (ModestEasysetupWizardDialog *self)
{
	ModestEasysetupWizardDialogPrivate *priv;

	priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);

	if (easysetup_provider_combo_box_get_active_provider_id (
		    EASYSETUP_PROVIDER_COMBO_BOX (priv->combo_account_serviceprovider)) == 0) {
		/* "Other..." was selected: */
		
		/* Make sure that the easysetup pages do not exist: */
		if(priv->page_complete_easysetup) {
			gtk_widget_destroy (priv->page_complete_easysetup);
			priv->page_complete_easysetup = NULL;
		}
		
		create_subsequent_customsetup_pages (self);
	}	
	else {
		/* A specific provider was selected: */
		{
			/* Make sure that the custom pages do not exist:
			 * Because they will be used if they exist, when creating the account. */
			if(priv->page_custom_incoming) {
				gtk_widget_destroy (priv->page_custom_incoming);
				priv->page_custom_incoming = NULL;
				priv->combo_incoming_servertype = NULL;
				priv->caption_incoming = NULL;
				priv->entry_incomingserver = NULL;
				priv->combo_incoming_security = NULL;
				priv->checkbox_incoming_auth = NULL;
			}
			
			if(priv->page_custom_outgoing) {
				gtk_widget_destroy (priv->page_custom_outgoing);
				priv->page_custom_outgoing = NULL;
				priv->entry_outgoingserver = NULL;
				priv->combo_outgoing_security = NULL;
				priv->combo_outgoing_auth = NULL;
				priv->checkbox_outgoing_smtp_specific = NULL;
				priv->button_outgoing_smtp_servers = NULL;
			}
			
			if(priv->page_complete_customsetup) {
				gtk_widget_destroy (priv->page_complete_customsetup);
				priv->page_complete_customsetup = NULL;
			}
		}
		
		/* Create the easysetup pages: */
		create_subsequent_easysetup_pages (self);
	}
}


static gchar*
util_get_default_servername_from_email_address (const gchar* email_address, ModestTransportStoreProtocol servertype)
{
	if (!email_address)
		return NULL;
	
	gchar* at = g_utf8_strchr (email_address, -1, '@');
	if (!at || (g_utf8_strlen (at, -1) < 2))
		return NULL;
		
	gchar* domain = g_utf8_next_char (at);
	if(!domain)
		return NULL;
		
	const gchar* hostname = NULL;
	if (servertype == MODEST_PROTOCOL_STORE_POP)
		hostname = "pop";
	else if (servertype == MODEST_PROTOCOL_STORE_IMAP)
		hostname = "imap";
	else if (servertype == MODEST_PROTOCOL_TRANSPORT_SMTP)
		hostname = "smtp";
	
	if (!hostname)
		return NULL;
		
	return g_strdup_printf ("%s.%s", hostname, domain);
}

static void 
set_default_custom_servernames (ModestEasysetupWizardDialog *self)
{
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(self);

	if (!priv->entry_incomingserver)
		return;
		
	/* Set a default domain for the server, based on the email address,
	 * if no server name was already specified.
	 */
	if (priv->entry_user_email
	    && ((priv->server_changes & MODEST_EASYSETUP_WIZARD_DIALOG_INCOMING_CHANGED) == 0)) {
		const ModestTransportStoreProtocol protocol = easysetup_servertype_combo_box_get_active_servertype (
			EASYSETUP_SERVERTYPE_COMBO_BOX (priv->combo_incoming_servertype));
		const gchar* email_address = gtk_entry_get_text (GTK_ENTRY(priv->entry_user_email));
		
		gchar* servername = util_get_default_servername_from_email_address (email_address, protocol);

		/* Do not set the INCOMING_CHANGED flag because of this edit */
		g_signal_handlers_block_by_func (G_OBJECT (priv->entry_incomingserver), G_CALLBACK (on_entry_incoming_servername_changed), self);
		gtk_entry_set_text (GTK_ENTRY (priv->entry_incomingserver), servername);
		g_signal_handlers_unblock_by_func (G_OBJECT (priv->entry_incomingserver), G_CALLBACK (on_entry_incoming_servername_changed), self);

		g_free (servername);
	}
	
	/* Set a default domain for the server, based on the email address,
	 * if no server name was already specified.
	 */
	if (!priv->entry_outgoingserver)
		return;
		
	if (priv->entry_user_email
	    && ((priv->server_changes & MODEST_EASYSETUP_WIZARD_DIALOG_OUTGOING_CHANGED) == 0)) {
		const gchar* email_address = gtk_entry_get_text (GTK_ENTRY(priv->entry_user_email));
		
		gchar* servername = util_get_default_servername_from_email_address (email_address, MODEST_PROTOCOL_TRANSPORT_SMTP);

		/* Do not set the OUTGOING_CHANGED flag because of this edit */
		g_signal_handlers_block_by_func (G_OBJECT (priv->entry_outgoingserver), G_CALLBACK (on_entry_outgoing_servername_changed), self);
		gtk_entry_set_text (GTK_ENTRY (priv->entry_outgoingserver), servername);
		g_signal_handlers_unblock_by_func (G_OBJECT (priv->entry_outgoingserver), G_CALLBACK (on_entry_outgoing_servername_changed), self);

		g_free (servername);
	}
}

static gchar*
get_entered_account_title (ModestEasysetupWizardDialog *self)
{
	ModestEasysetupWizardDialogPrivate *priv;
	const gchar* account_title;

	priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE(self);
	account_title = gtk_entry_get_text (GTK_ENTRY (priv->entry_account_title));

	if (!account_title || (strlen (account_title) == 0)) {
		return NULL;
	} else {
		/* Strip it of whitespace at the start and end: */
		gchar *result = g_strdup (account_title);
		result = g_strstrip (result);
		
		if (!result)
			return NULL;
			
		if (strlen (result) == 0) {
			g_free (result);
			return NULL;	
		}
		
		return result;
	}
}

static gboolean
on_before_next (ModestWizardDialog *dialog, GtkWidget *current_page, GtkWidget *next_page)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (dialog);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);

	/* if are browsing pages previous to the last one, then we have pending settings in
	 * this wizard */
	if (next_page != NULL)
		priv->pending_load_settings = TRUE;
	
	/* Do extra validation that couldn't be done for every key press,
	 * either because it was too slow,
	 * or because it requires interaction:
	 */
	if (current_page == priv->page_account_details) {	
		/* Check that the title is not already in use: */
		gchar* account_title = get_entered_account_title (self);
		if (!account_title)
			return FALSE;
			
		/* Aavoid a clash with an existing display name: */
		const gboolean name_in_use = modest_account_mgr_account_with_display_name_exists (
			priv->account_manager, account_title);

		if (name_in_use) {
			/* Warn the user via a dialog: */
			hildon_banner_show_information(NULL, NULL, _("mail_ib_account_name_already_existing"));
            
			return FALSE;
		}
	}
	else if (current_page == priv->page_user_details) {
		/* Check that the email address is valud: */
		const gchar* email_address = gtk_entry_get_text (GTK_ENTRY (priv->entry_user_email));
		if ((!email_address) || (strlen(email_address) == 0))
			return FALSE;
			
		if (!modest_text_utils_validate_email_address (email_address, NULL)) {
			/* Warn the user via a dialog: */
			hildon_banner_show_information (NULL, NULL, _("mcen_ib_invalid_email"));
                                             
			/* Return focus to the email address entry: */
			gtk_widget_grab_focus (priv->entry_user_email);
			gtk_editable_select_region (GTK_EDITABLE (priv->entry_user_email), 0, -1);

			return FALSE;
		}
		
		/* Make sure that the subsequent pages are appropriate for the provider choice. */
		create_subsequent_pages (self);
	}
	
	/* TODO: The UI Spec wants us to check that the servernames are valid, 
	 * but does not specify how.
	 */
	  
	if(next_page == priv->page_custom_incoming) {
		set_default_custom_servernames (self);
	}
	else if (next_page == priv->page_custom_outgoing) {
		set_default_custom_servernames (self);

		/* Check if the server supports secure authentication */
		const ModestConnectionProtocol security_incoming = 
			modest_serversecurity_combo_box_get_active_serversecurity (
				MODEST_SERVERSECURITY_COMBO_BOX (priv->combo_incoming_security));
		if (gtk_toggle_button_get_active (
			GTK_TOGGLE_BUTTON (priv->checkbox_incoming_auth))
				&& !modest_protocol_info_is_secure(security_incoming))
		{
				if (!check_has_supported_auth_methods (self))
					return FALSE;
		}
	}
	
	/* If this is the last page, and this is a click on Finish, 
	 * then attempt to create the dialog.
	 */
	if(!next_page) /* This is NULL when this is a click on Finish. */
	{
		if (priv->pending_load_settings) {
			save_to_settings (self);
		}
		modest_account_mgr_add_account_from_settings (priv->account_manager, priv->settings);
	}
	
	
	return TRUE;
}

static gboolean entry_is_empty (GtkWidget *entry)
{
	if (!entry)
		return FALSE;
		
	const gchar* text = gtk_entry_get_text (GTK_ENTRY (entry));
	if ((!text) || (strlen(text) == 0))
		return TRUE;
	else {
		/* Strip it of whitespace at the start and end: */
		gchar *stripped = g_strdup (text);
		stripped = g_strstrip (stripped);
		
		if (!stripped)
			return TRUE;
			
		const gboolean result = (strlen (stripped) == 0);
		
		g_free (stripped);
		return result;
	}
}

static void
on_enable_buttons (ModestWizardDialog *dialog, GtkWidget *current_page)
{
	ModestEasysetupWizardDialog *self = MODEST_EASYSETUP_WIZARD_DIALOG (dialog);
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	
	gboolean enable_next = TRUE;
	if (current_page == priv->page_welcome) {
		enable_next = TRUE;
	}
	else if (current_page == priv->page_account_details) {
		/* The account details title is mandatory: */
		if (entry_is_empty(priv->entry_account_title))
			enable_next = FALSE;
	}
	else if (current_page == priv->page_user_details) {	
		/* The user details username is mandatory: */
		if (entry_is_empty(priv->entry_user_username))
			enable_next = FALSE;
			
		/* The user details email address is mandatory: */
		if (enable_next && entry_is_empty (priv->entry_user_email))
			enable_next = FALSE;
	}
	else if (current_page == priv->page_custom_incoming) {
		/* The custom incoming server is mandatory: */
		if (entry_is_empty(priv->entry_incomingserver))
			enable_next = FALSE;
	}
			
	/* Enable the buttons, 
	 * identifying them via their associated response codes: */
	                           
	/* Disable the Finish button until we are on the last page,
	 * because HildonWizardDialog enables this for all but the first page: */
	GtkNotebook *notebook = NULL;
	GtkDialog *dialog_base = GTK_DIALOG (dialog);
	g_object_get (dialog_base, "wizard-notebook", &notebook, NULL);
	
	gint current = gtk_notebook_get_current_page (notebook);
	gint last = gtk_notebook_get_n_pages (notebook) - 1;
	const gboolean is_last = (current == last);
    
	if(!is_last) {
		gtk_dialog_set_response_sensitive (dialog_base,
						   MODEST_WIZARD_DIALOG_FINISH,
						   FALSE);
	} else
	{
		/* Disable Next on the last page: */
		enable_next = FALSE;
	}
    	
	gtk_dialog_set_response_sensitive (dialog_base,
					   MODEST_WIZARD_DIALOG_NEXT,
					   enable_next);
}

static void
modest_easysetup_wizard_dialog_class_init (ModestEasysetupWizardDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	g_type_class_add_private (klass, sizeof (ModestEasysetupWizardDialogPrivate));


	object_class->get_property = modest_easysetup_wizard_dialog_get_property;
	object_class->set_property = modest_easysetup_wizard_dialog_set_property;
	object_class->dispose = modest_easysetup_wizard_dialog_dispose;
	object_class->finalize = modest_easysetup_wizard_dialog_finalize;
	
	/* Provide a vfunc implementation so we can decide 
	 * when to enable/disable the prev/next buttons.
	 */
	ModestWizardDialogClass *base_klass = (ModestWizardDialogClass*)(klass);
	base_klass->before_next = on_before_next;
	base_klass->enable_buttons = on_enable_buttons;
}
 
static void
show_error (GtkWidget *parent_widget, const gchar* text)
{
	//TODO: Apparently this doesn't show anything in Maemo Bora:
	hildon_banner_show_information(parent_widget, NULL, text);	
}


/**
 * save_to_settings:
 * @self: a #ModestEasysetupWizardDialog
 *
 * takes information from all the wizard and stores it in settings
 */
static void
save_to_settings (ModestEasysetupWizardDialog *self)
{
	ModestEasysetupWizardDialogPrivate *priv = MODEST_EASYSETUP_WIZARD_DIALOG_GET_PRIVATE (self);
	guint special_port;
	gchar *provider_id;
	gchar* display_name;
	const gchar *username, *password;
	gchar *store_hostname, *transport_hostname;
	guint store_port, transport_port;
	ModestTransportStoreProtocol store_protocol, transport_protocol;
	ModestConnectionProtocol store_security, transport_security;
	ModestAuthProtocol store_auth_protocol, transport_auth_protocol;
	ModestServerAccountSettings *store_settings, *transport_settings;
	const gchar *fullname, *email_address;

	/* Get details from the specified presets: */
	provider_id = easysetup_provider_combo_box_get_active_provider_id (
		EASYSETUP_PROVIDER_COMBO_BOX (priv->combo_account_serviceprovider));
		
	/* username and password (for both incoming and outgoing): */
	username = gtk_entry_get_text (GTK_ENTRY (priv->entry_user_username));
	password = gtk_entry_get_text (GTK_ENTRY (priv->entry_user_password));

	/* Incoming server: */
	/* Note: We need something as default for the ModestTransportStoreProtocol* values, 
	 * or modest_account_mgr_add_server_account will fail. */
	store_port = 0;
	store_protocol = MODEST_PROTOCOL_STORE_POP;
	store_security = MODEST_PROTOCOL_CONNECTION_NORMAL;
	store_auth_protocol = MODEST_PROTOCOL_AUTH_NONE;

	if (provider_id) {
		ModestPresetsServerType store_provider_server_type;
		ModestPresetsSecurity store_provider_security;
		/* Use presets: */
		store_hostname = modest_presets_get_server (priv->presets, provider_id, 
							    TRUE /* store */);
		
		store_provider_server_type = modest_presets_get_info_server_type (priv->presets,
									 provider_id, 
									 TRUE /* store */);
		store_provider_security  = modest_presets_get_info_server_security (priv->presets,
										    provider_id, 
										    TRUE /* store */);

		/* We don't check for SMTP here as that is impossible for an incoming server. */
		if (store_provider_server_type == MODEST_PRESETS_SERVER_TYPE_IMAP)
			store_protocol = MODEST_PROTOCOL_STORE_IMAP;
		else if (store_provider_server_type == MODEST_PRESETS_SERVER_TYPE_POP)
			store_protocol = MODEST_PROTOCOL_STORE_POP;
		else /* fallback */
			store_protocol = MODEST_PROTOCOL_STORE_POP;

		/* we check if there is a *special* port */
		special_port = modest_presets_get_port (priv->presets, provider_id, TRUE /* incoming */);
		if (special_port != 0)
			store_port = special_port;
		else 
			store_port = get_serverport_incoming(store_provider_server_type, store_provider_security);
		
		if (store_provider_security & MODEST_PRESETS_SECURITY_SECURE_INCOMING)
			store_security = MODEST_PROTOCOL_CONNECTION_SSL; /* TODO: Is this what we want? */
		
		if (store_provider_security & MODEST_PRESETS_SECURITY_APOP)
			store_auth_protocol = MODEST_PROTOCOL_AUTH_PASSWORD; /* TODO: Is this what we want? */
	} else {
		/* Use custom pages because no preset was specified: */
		store_hostname = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry_incomingserver) ));		
		store_protocol = easysetup_servertype_combo_box_get_active_servertype (
			EASYSETUP_SERVERTYPE_COMBO_BOX (priv->combo_incoming_servertype));		
		store_security = modest_serversecurity_combo_box_get_active_serversecurity (
			MODEST_SERVERSECURITY_COMBO_BOX (priv->combo_incoming_security));
		
		/* The UI spec says: 
		 * If secure authentication is unchecked, allow sending username and password also as plain text.
		 * If secure authentication is checked, require one of the secure methods during 
		 * connection: SSL, TLS, CRAM-MD5 etc. */
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbox_incoming_auth)) &&
		    !modest_protocol_info_is_secure(store_security)) {
			store_auth_protocol = check_first_supported_auth_method (self);
		} else {
			store_auth_protocol = MODEST_PROTOCOL_AUTH_PASSWORD;
		}
	}

	/* now we store the store account settings */
	store_settings = modest_account_settings_get_store_settings (priv->settings);
	modest_server_account_settings_set_hostname (store_settings, store_hostname);
	modest_server_account_settings_set_username (store_settings, username);
	modest_server_account_settings_set_password (store_settings, password);
	modest_server_account_settings_set_protocol (store_settings, store_protocol);
	modest_server_account_settings_set_security (store_settings, store_security);
	modest_server_account_settings_set_auth_protocol (store_settings, store_auth_protocol);
	if (store_port != 0)
		modest_server_account_settings_set_port (store_settings, store_port);

	g_object_unref (store_settings);
	g_free (store_hostname);
	
	/* Outgoing server: */
	transport_hostname = NULL;
	transport_protocol = MODEST_PROTOCOL_STORE_POP;
	transport_security = MODEST_PROTOCOL_CONNECTION_NORMAL;
	transport_auth_protocol = MODEST_PROTOCOL_AUTH_NONE;
	transport_port = 0;
	
	if (provider_id) {
		ModestPresetsServerType transport_provider_server_type;
		ModestPresetsSecurity transport_provider_security;

		/* Use presets: */
		transport_hostname = modest_presets_get_server (priv->presets, provider_id, 
								FALSE /* transport */);
			
		transport_provider_server_type = modest_presets_get_info_server_type (priv->presets,
										      provider_id, 
										      FALSE /* transport */);		
		transport_provider_security = modest_presets_get_info_server_security (priv->presets, 
										       provider_id, 
										       FALSE /* transport */);

		/* Note: We need something as default, or modest_account_mgr_add_server_account will fail. */
		transport_protocol = MODEST_PROTOCOL_TRANSPORT_SENDMAIL; 
		if (transport_provider_server_type == MODEST_PRESETS_SERVER_TYPE_SMTP)
			transport_protocol = MODEST_PROTOCOL_TRANSPORT_SMTP;

		transport_security = MODEST_PROTOCOL_CONNECTION_NORMAL;
		if (transport_provider_security & MODEST_PRESETS_SECURITY_SECURE_SMTP) {
			/* printf("DEBUG: %s: using secure SMTP\n", __FUNCTION__); */
			transport_security = MODEST_PROTOCOL_CONNECTION_SSL; /* TODO: Is this what we want? */
			/* we check if there is a *special* port */
			special_port = modest_presets_get_port (priv->presets, provider_id,
								FALSE /* transport */);
			if (special_port != 0)
				transport_port = special_port;
			else 
				transport_port = 465;
			transport_auth_protocol = MODEST_PROTOCOL_AUTH_PASSWORD;
		} else {
			/* printf("DEBUG: %s: using non-secure SMTP\n", __FUNCTION__); */
			transport_auth_protocol = MODEST_PROTOCOL_AUTH_NONE;
		}
	} else {
		/* Use custom pages because no preset was specified: */
		transport_hostname = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry_outgoingserver) ));
		transport_protocol = MODEST_PROTOCOL_TRANSPORT_SMTP; /* It's always SMTP for outgoing. */
		transport_security = modest_serversecurity_combo_box_get_active_serversecurity (
			MODEST_SERVERSECURITY_COMBO_BOX (priv->combo_outgoing_security));		
		transport_auth_protocol = modest_secureauth_combo_box_get_active_secureauth (
			MODEST_SECUREAUTH_COMBO_BOX (priv->combo_outgoing_auth));
	}
	    
	/* now we transport the transport account settings */
	transport_settings = modest_account_settings_get_transport_settings (priv->settings);
	modest_server_account_settings_set_hostname (transport_settings, transport_hostname);
	modest_server_account_settings_set_username (transport_settings, username);
	modest_server_account_settings_set_password (transport_settings, password);
	modest_server_account_settings_set_protocol (transport_settings, transport_protocol);
	modest_server_account_settings_set_security (transport_settings, transport_security);
	modest_server_account_settings_set_auth_protocol (transport_settings, transport_auth_protocol);
	if (transport_port != 0)
		modest_server_account_settings_set_port (transport_settings, transport_port);

	g_object_unref (transport_settings);
	g_free (transport_hostname);
	
	fullname = gtk_entry_get_text (GTK_ENTRY (priv->entry_user_name));
	email_address = gtk_entry_get_text (GTK_ENTRY (priv->entry_user_email));
	modest_account_settings_set_fullname (priv->settings, fullname);
	modest_account_settings_set_email_address (priv->settings, email_address);
	/* we don't set retrieve type to preserve advanced settings if any. By default account settings
	   are set to headers only */
	
	/* Save the connection-specific SMTP server accounts. */
        modest_account_settings_set_use_connection_specific_smtp 
		(priv->settings, 
		 gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->checkbox_outgoing_smtp_specific)));

	display_name = get_entered_account_title (self);
	modest_account_settings_set_display_name (priv->settings, display_name);
	g_free (display_name);

	if (priv->specific_window)
		modest_connection_specific_smtp_window_save_server_accounts (
			MODEST_CONNECTION_SPECIFIC_SMTP_WINDOW (priv->specific_window));
}
