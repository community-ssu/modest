/* Copyright (c) 2007, Nokia Corporation
 * All rights reserved.
 *
 */


#include "modest-account-settings-dialog.h"
#include <glib/gi18n.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkstock.h>
#include <hildon-widgets/hildon-caption.h>
#include "maemo/easysetup/modest-easysetup-country-combo-box.h"
#include "maemo/easysetup/modest-easysetup-provider-combo-box.h"
#include "maemo/easysetup/modest-easysetup-servertype-combo-box.h"
#include "maemo/easysetup/modest-easysetup-serversecurity-combo-box.h"
#include "maemo/easysetup/modest-easysetup-secureauth-combo-box.h"
#include "maemo/easysetup/modest-validating-entry.h"
#include "modest-text-utils.h"
#include "modest-account-mgr.h"
#include "modest-account-mgr-helpers.h" /* For modest_account_mgr_get_account_data(). */
#include "modest-runtime.h" /* For modest_runtime_get_account_mgr(). */
#include "maemo/modest-connection-specific-smtp-window.h"
#include <gconf/gconf-client.h>
#include <string.h> /* For strlen(). */

/* Include config.h so that _() works: */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define EXAMPLE_EMAIL_ADDRESS "first.last@provider.com"

G_DEFINE_TYPE (ModestAccountSettingsDialog, modest_account_settings_dialog, GTK_TYPE_DIALOG);

#define ACCOUNT_SETTINGS_DIALOG_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), MODEST_TYPE_ACCOUNT_SETTINGS_DIALOG, ModestAccountSettingsDialogPrivate))

typedef struct _ModestAccountSettingsDialogPrivate ModestAccountSettingsDialogPrivate;

struct _ModestAccountSettingsDialogPrivate
{
};

static void
enable_buttons (ModestAccountSettingsDialog *self);

static void
save_configuration (ModestAccountSettingsDialog *dialog);

static void
modest_account_settings_dialog_get_property (GObject *object, guint property_id,
															GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
modest_account_settings_dialog_set_property (GObject *object, guint property_id,
															const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
modest_account_settings_dialog_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (modest_account_settings_dialog_parent_class)->dispose)
		G_OBJECT_CLASS (modest_account_settings_dialog_parent_class)->dispose (object);
}

static void
modest_account_settings_dialog_finalize (GObject *object)
{
	ModestAccountSettingsDialog *self = MODEST_ACCOUNT_SETTINGS_DIALOG (object);
	
	if (self->original_account_name)
		g_free (self->original_account_name);
		
	if (self->account_manager)
		g_object_unref (G_OBJECT (self->account_manager));
	
	G_OBJECT_CLASS (modest_account_settings_dialog_parent_class)->finalize (object);
}

#if 0
static void
show_error (GtkWindow *parent_window, const gchar* text);
#endif


static void
on_caption_entry_changed (GtkEditable *editable, gpointer user_data)
{
	ModestAccountSettingsDialog *self = MODEST_ACCOUNT_SETTINGS_DIALOG (user_data);
	g_assert(self);
	enable_buttons(self);
}

static void
on_caption_combobox_changed (GtkComboBox *widget, gpointer user_data)
{
	ModestAccountSettingsDialog *self = MODEST_ACCOUNT_SETTINGS_DIALOG (user_data);
	g_assert(self);
	enable_buttons(self);
}

/** This is a convenience function to create a caption containing a mandatory widget.
 * When the widget is edited, the enable_buttons() vfunc will be called.
 */
static GtkWidget* create_caption_new_with_asterix(ModestAccountSettingsDialog *self,
	GtkSizeGroup *group,
	const gchar *value,
	GtkWidget *control,
	GtkWidget *icon,
	HildonCaptionStatus flag)
{
  GtkWidget *caption = hildon_caption_new (group, value, control, icon, flag);
  
/* The translated strings seem to already contain the *,
 * but this code can be used if that is not true in future.
 */
#if 0
	/* Add a * character to indicate mandatory fields,
	 * as specified in our "Email UI Specification": */
	if (flag == HILDON_CAPTION_MANDATORY) {
		gchar* title = g_strdup_printf("%s*", value);
		caption = hildon_caption_new (group, title, control, icon, flag);	
		g_free(title);
	}	
	else
		caption = hildon_caption_new (group, value, control, icon, flag);
#endif

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
create_page_account_details (ModestAccountSettingsDialog *self)
{
	GtkWidget *box = gtk_vbox_new (FALSE, 2);
	GtkWidget *label = gtk_label_new(_("mcen_ia_accountdetails"));
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 2);
	gtk_widget_show (label);
	
	/* Create a size group to be used by all captions.
	 * Note that HildonCaption does not create a default size group if we do not specify one.
	 * We use GTK_SIZE_GROUP_HORIZONTAL, so that the widths are the same. */
	GtkSizeGroup* sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
           
	/* The description widgets: */	
	self->entry_account_title = GTK_WIDGET (easysetup_validating_entry_new ());
	GtkWidget *caption = create_caption_new_with_asterix (self, sizegroup, _("mcen_fi_account_title"), 
		self->entry_account_title, NULL, HILDON_CAPTION_MANDATORY);
	gtk_widget_show (self->entry_account_title);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
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
	easysetup_validating_entry_set_unallowed_characters (
	 	EASYSETUP_VALIDATING_ENTRY (self->entry_account_title), list_prevent);
	g_list_free (list_prevent);
	
	/* Set max length as in the UI spec:
	 * TODO: The UI spec seems to want us to show a dialog if we hit the maximum. */
	gtk_entry_set_max_length (GTK_ENTRY (self->entry_account_title), 64);
	
	/* TODO: The Retrieve and Limit Retrieve combo boxes. */
	
	/* The leave-messages widgets: */
	if(!self->checkbox_leave_messages)
		self->checkbox_leave_messages = 
			gtk_check_button_new_with_label (_("mcen_fi_advsetup_leave_on_server"));
	gtk_box_pack_start (GTK_BOX (box), self->checkbox_leave_messages, FALSE, FALSE, 2);
	gtk_widget_show (self->checkbox_leave_messages);
	
	
	gtk_widget_show (GTK_WIDGET (box));
	
	return GTK_WIDGET (box);
}

static void
on_button_signature (GtkButton *button, gpointer user_data)
{
	
}

static GtkWidget*
create_page_user_details (ModestAccountSettingsDialog *self)
{
	GtkWidget *box = gtk_vbox_new (FALSE, 2);
	
	/* Create a size group to be used by all captions.
	 * Note that HildonCaption does not create a default size group if we do not specify one.
	 * We use GTK_SIZE_GROUP_HORIZONTAL, so that the widths are the same. */
	GtkSizeGroup* sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	 
	/* The name widgets: */
	self->entry_user_name = GTK_WIDGET (easysetup_validating_entry_new ());
	/* Set max length as in the UI spec:
	 * TODO: The UI spec seems to want us to show a dialog if we hit the maximum. */
	gtk_entry_set_max_length (GTK_ENTRY (self->entry_user_name), 64);
	GtkWidget *caption = create_caption_new_with_asterix (self, sizegroup, 
		_("mcen_li_emailsetup_name"), self->entry_user_name, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->entry_user_name);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
	
	/* Prevent the use of some characters in the name, 
	 * as required by our UI specification: */
	GList *list_prevent = NULL;
	list_prevent = g_list_append (list_prevent, "<");
	list_prevent = g_list_append (list_prevent, ">");
	easysetup_validating_entry_set_unallowed_characters (
	 	EASYSETUP_VALIDATING_ENTRY (self->entry_user_name), list_prevent);
	g_list_free (list_prevent);
	
	/* The username widgets: */	
	self->entry_user_username = GTK_WIDGET (easysetup_validating_entry_new ());
	caption = create_caption_new_with_asterix (self, sizegroup, _("mail_fi_username"), 
		self->entry_user_username, NULL, HILDON_CAPTION_MANDATORY);
	gtk_widget_show (self->entry_user_username);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
	
	/* Prevent the use of some characters in the username, 
	 * as required by our UI specification: */
	easysetup_validating_entry_set_unallowed_characters_whitespace (
	 	EASYSETUP_VALIDATING_ENTRY (self->entry_user_username));
	
	/* Set max length as in the UI spec:
	 * TODO: The UI spec seems to want us to show a dialog if we hit the maximum. */
	gtk_entry_set_max_length (GTK_ENTRY (self->entry_user_username), 64);
	
	/* The password widgets: */	
	self->entry_user_password = gtk_entry_new ();
	gtk_entry_set_visibility (GTK_ENTRY (self->entry_user_password), FALSE);
	/* gtk_entry_set_invisible_char (GTK_ENTRY (self->entry_user_password), '*'); */
	caption = create_caption_new_with_asterix (self, sizegroup, 
		_("mail_fi_password"), self->entry_user_password, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->entry_user_password);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
	
	/* The email address widgets: */	
	self->entry_user_email = GTK_WIDGET (easysetup_validating_entry_new ());
	caption = create_caption_new_with_asterix (self, sizegroup, 
		_("mcen_li_emailsetup_email_address"), self->entry_user_email, NULL, HILDON_CAPTION_MANDATORY);
	gtk_entry_set_text (GTK_ENTRY (self->entry_user_email), EXAMPLE_EMAIL_ADDRESS); /* Default text. */
	gtk_widget_show (self->entry_user_email);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
	
	/* Set max length as in the UI spec:
	 * TODO: The UI spec seems to want us to show a dialog if we hit the maximum. */
	gtk_entry_set_max_length (GTK_ENTRY (self->entry_user_email), 64);
	
	
	/* Signature button: */
	if (!self->button_signature)
		self->button_signature = gtk_button_new_with_label (_("mcen_bd_emailsetup_edit"));
	caption = hildon_caption_new (sizegroup, _("mcen_fi_email_signature"), 
		self->button_signature, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->button_signature);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
		
	g_signal_connect (G_OBJECT (self->button_signature), "clicked",
        	G_CALLBACK (on_button_signature), self);
        	
	gtk_widget_show (GTK_WIDGET (box));
	
	return GTK_WIDGET (box);
}

/** Change the caption title for the incoming server, 
 * as specified in the UI spec:
 */
static void update_incoming_server_title (ModestAccountSettingsDialog *self, ModestProtocol protocol)
{
	const gchar* type = 
		(protocol == MODEST_PROTOCOL_STORE_POP ? 
			_("mail_fi_emailtype_pop3") : 
			_("mail_fi_emailtype_imap") );
			
		
	/* Note that this produces a compiler warning, 
	 * because the compiler does not know that the translated string will have a %s in it.
	 * I do not see a way to avoid the warning while still using these Logical IDs. murrayc. */
	gchar* incomingserver_title = g_strdup_printf(_("mcen_li_emailsetup_servertype"), type);
	g_object_set (G_OBJECT (self->caption_incoming), "label", incomingserver_title, NULL);
	g_free(incomingserver_title);
}

/** Change the caption title for the incoming server, 
 * as specified in the UI spec:
 */
static void update_incoming_server_security_choices (ModestAccountSettingsDialog *self, ModestProtocol protocol)
{
	/* Fill the combo with appropriately titled choices for POP or IMAP. */
	/* The choices are the same, but the titles are different, as in the UI spec. */
	easysetup_serversecurity_combo_box_fill (
		EASYSETUP_SERVERSECURITY_COMBO_BOX (self->combo_incoming_security), protocol);
}
           
static GtkWidget* create_page_incoming (ModestAccountSettingsDialog *self)
{
	GtkWidget *box = gtk_vbox_new (FALSE, 2);
	
	/* Create a size group to be used by all captions.
	 * Note that HildonCaption does not create a default size group if we do not specify one.
	 * We use GTK_SIZE_GROUP_HORIZONTAL, so that the widths are the same. */
	GtkSizeGroup *sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	 
	/* The incoming server widgets: */
	if(!self->entry_incomingserver)
		self->entry_incomingserver = gtk_entry_new ();

	if (self->caption_incoming)
	  gtk_widget_destroy (self->caption_incoming);
	   
	/* The caption title will be updated in update_incoming_server_title().
	 * so this default text will never be seen: */
	/* (Note: Changing the title seems pointless. murrayc) */
	self->caption_incoming = create_caption_new_with_asterix (self, sizegroup, 
		"Incoming Server", self->entry_incomingserver, NULL, HILDON_CAPTION_MANDATORY);
	gtk_widget_show (self->entry_incomingserver);
	gtk_box_pack_start (GTK_BOX (box), self->caption_incoming, FALSE, FALSE, 2);
	gtk_widget_show (self->caption_incoming);
	
	/* The secure connection widgets: */
	/* This will be filled by update_incoming_server_security_choices(). */
	if (!self->combo_incoming_security)
		self->combo_incoming_security = GTK_WIDGET (easysetup_serversecurity_combo_box_new ());
	easysetup_serversecurity_combo_box_set_active_serversecurity (
		EASYSETUP_SERVERSECURITY_COMBO_BOX (self->combo_incoming_security), MODEST_PROTOCOL_SECURITY_NONE);
	GtkWidget *caption = hildon_caption_new (sizegroup, _("mcen_li_emailsetup_secure_connection"), 
		self->combo_incoming_security, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->combo_incoming_security);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
	
	/* The port widgets: */
	if (!self->entry_incoming_port)
		self->entry_incoming_port = GTK_WIDGET (gtk_entry_new ());
	caption = hildon_caption_new (sizegroup, _("mcen_fi_emailsetup_port"), 
		self->entry_incoming_port, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->entry_incoming_port);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
	
	/* The secure authentication widgets: */
	if(!self->checkbox_incoming_auth)
		self->checkbox_incoming_auth = 
			gtk_check_button_new_with_label (_("mcen_li_emailsetup_secure_authentication"));
	gtk_box_pack_start (GTK_BOX (box), self->checkbox_incoming_auth, FALSE, FALSE, 2);
	gtk_widget_show (self->checkbox_incoming_auth);
	
	gtk_widget_show (GTK_WIDGET (box));
	
	return GTK_WIDGET (box);
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
on_smtp_servers_window_hide (GtkWindow *window, gpointer user_data)
{
	/* Destroy the window when it is closed: */
	gtk_widget_destroy (GTK_WIDGET (window));
}

static void
on_button_outgoing_smtp_servers (GtkButton *button, gpointer user_data)
{

	ModestAccountSettingsDialog * self = MODEST_ACCOUNT_SETTINGS_DIALOG (user_data);
	
	/* Show the window: */
	/* TODO: Retrieve the chosen settings,
	 * so we can supply them when creating the connection somehow.
	 */
	GtkWidget *window = GTK_WIDGET (modest_connection_specific_smtp_window_new ());
	gtk_window_set_transient_for (GTK_WINDOW (self), GTK_WINDOW (window));
	g_signal_connect (G_OBJECT (window), "hide",
        	G_CALLBACK (on_smtp_servers_window_hide), self);
    gtk_widget_show (window);
}

static void
on_combo_outgoing_auth_changed (GtkComboBox *widget, gpointer user_data)
{
	ModestAccountSettingsDialog *self = MODEST_ACCOUNT_SETTINGS_DIALOG (user_data);
	
	ModestProtocol protocol_security = 
		easysetup_secureauth_combo_box_get_active_secureauth (
			EASYSETUP_SECUREAUTH_COMBO_BOX (self->combo_outgoing_auth));
	const gboolean secureauth_used = protocol_security != MODEST_PROTOCOL_AUTH_NONE;
	
	gtk_widget_set_sensitive (self->caption_outgoing_username, secureauth_used);
	gtk_widget_set_sensitive (self->caption_outgoing_password, secureauth_used);
}

static GtkWidget* create_page_outgoing (ModestAccountSettingsDialog *self)
{
	GtkWidget *box = gtk_vbox_new (FALSE, 2);
	
	/* Create a size group to be used by all captions.
	 * Note that HildonCaption does not create a default size group if we do not specify one.
	 * We use GTK_SIZE_GROUP_HORIZONTAL, so that the widths are the same. */
	GtkSizeGroup *sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	 
	/* The outgoing server widgets: */
	if (!self->entry_outgoingserver)
		self->entry_outgoingserver = gtk_entry_new ();
	GtkWidget *caption = create_caption_new_with_asterix (self, sizegroup, 
		_("mcen_li_emailsetup_smtp"), self->entry_outgoingserver, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->entry_outgoingserver);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
	
	/* The secure authentication widgets: */
	if (!self->combo_outgoing_auth)
		self->combo_outgoing_auth = GTK_WIDGET (easysetup_secureauth_combo_box_new ());
	caption = hildon_caption_new (sizegroup, _("mcen_li_emailsetup_secure_authentication"), 
		self->combo_outgoing_auth, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->combo_outgoing_auth);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
	
	/* Dim the outgoing username and password when no secure authentication is used, as per the UI spec: */
	g_signal_connect (G_OBJECT (self->combo_outgoing_auth), "changed", (GCallback)on_combo_outgoing_auth_changed, self);
	
	/* The username widgets: */	
	self->entry_outgoing_username = GTK_WIDGET (easysetup_validating_entry_new ());
	self->caption_outgoing_username = create_caption_new_with_asterix (self, sizegroup, _("mail_fi_username"), 
		self->entry_outgoing_username, NULL, HILDON_CAPTION_MANDATORY);
	gtk_widget_show (self->entry_outgoing_username);
	gtk_box_pack_start (GTK_BOX (box), self->caption_outgoing_username, FALSE, FALSE, 2);
	gtk_widget_show (self->caption_outgoing_username);
	
	/* Prevent the use of some characters in the username, 
	 * as required by our UI specification: */
	easysetup_validating_entry_set_unallowed_characters_whitespace (
	 	EASYSETUP_VALIDATING_ENTRY (self->entry_outgoing_username));
	
	/* Set max length as in the UI spec:
	 * TODO: The UI spec seems to want us to show a dialog if we hit the maximum. */
	gtk_entry_set_max_length (GTK_ENTRY (self->entry_outgoing_username), 64);
	
	/* The password widgets: */	
	self->entry_outgoing_password = gtk_entry_new ();
	gtk_entry_set_visibility (GTK_ENTRY (self->entry_outgoing_password), FALSE);
	/* gtk_entry_set_invisible_char (GTK_ENTRY (self->entry_outgoing_password), '*'); */
	self->caption_outgoing_password = create_caption_new_with_asterix (self, sizegroup, 
		_("mail_fi_password"), self->entry_outgoing_password, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->entry_outgoing_password);
	gtk_box_pack_start (GTK_BOX (box), self->caption_outgoing_password, FALSE, FALSE, 2);
	gtk_widget_show (self->caption_outgoing_password);
	
	/* The secure connection widgets: */
	/* This will be filled and set with easysetup_serversecurity_combo_box_fill() 
	 * and easysetup_serversecurity_combo_box_set_active_serversecurity().
	 */
	if (!self->combo_outgoing_security)
		self->combo_outgoing_security = GTK_WIDGET (easysetup_serversecurity_combo_box_new ());
	caption = hildon_caption_new (sizegroup, _("mcen_li_emailsetup_secure_connection"), 
		self->combo_outgoing_security, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->combo_outgoing_security);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);	
	
	/* The port widgets: */
	if (!self->entry_outgoing_port)
		self->entry_outgoing_port = GTK_WIDGET (gtk_entry_new ());
	caption = hildon_caption_new (sizegroup, _("mcen_fi_emailsetup_port"), 
		self->entry_outgoing_port, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->entry_outgoing_port);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
	
	/* connection-specific checkbox: */
	if (!self->checkbox_outgoing_smtp_specific) {
		self->checkbox_outgoing_smtp_specific = gtk_check_button_new_with_label (_("mcen_fi_advsetup_connection_smtp"));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self->checkbox_outgoing_smtp_specific), 
			FALSE);
	}
	gtk_box_pack_start (GTK_BOX (box), self->checkbox_outgoing_smtp_specific, FALSE, FALSE, 2);
	gtk_widget_show (self->checkbox_outgoing_smtp_specific);
	
	/* Connection-specific SMTP-Severs Edit button: */
	if (!self->button_outgoing_smtp_servers)
		self->button_outgoing_smtp_servers = gtk_button_new_with_label (_("mcen_bd_emailsetup_edit"));
	caption = hildon_caption_new (sizegroup, _("mcen_fi_advsetup_optional_smtp"), 
		self->button_outgoing_smtp_servers, NULL, HILDON_CAPTION_OPTIONAL);
	gtk_widget_show (self->button_outgoing_smtp_servers);
	gtk_box_pack_start (GTK_BOX (box), caption, FALSE, FALSE, 2);
	gtk_widget_show (caption);
	
	/* Only enable the button when the checkbox is checked: */
	enable_widget_for_togglebutton (self->button_outgoing_smtp_servers, 
		GTK_TOGGLE_BUTTON (self->checkbox_outgoing_smtp_specific));
		
	g_signal_connect (G_OBJECT (self->button_outgoing_smtp_servers), "clicked",
        	G_CALLBACK (on_button_outgoing_smtp_servers), self);
	
	
	gtk_widget_show (GTK_WIDGET (box));
	
	return GTK_WIDGET (box);
}

/*
 */
static void 
on_response (GtkDialog *wizard_dialog,
	gint response_id,
	gpointer user_data)
{
	ModestAccountSettingsDialog *self = MODEST_ACCOUNT_SETTINGS_DIALOG (wizard_dialog);
	enable_buttons (self);
	
	/* TODO: Prevent the OK response if the data is invalid. */
	
	if (response_id == GTK_RESPONSE_OK)
	  save_configuration (self);
}

static void
modest_account_settings_dialog_init (ModestAccountSettingsDialog *self)
{
	/* Create the notebook to be used by the GtkDialog base class:
	 * Each page of the notebook will be a page of the wizard: */
	GtkNotebook *notebook = GTK_NOTEBOOK (gtk_notebook_new());

    
    gtk_window_set_title (GTK_WINDOW (self), _("mcen_ti_emailsetup"));
	
	/* Get the account manager object, 
	 * so we can check for existing accounts,
	 * and create new accounts: */
	self->account_manager = modest_runtime_get_account_mgr ();
	g_assert (self->account_manager);
	g_object_ref (self->account_manager);
	
    /* Create the common pages, 
     */
	self->page_account_details = create_page_account_details (self);
	self->page_user_details = create_page_user_details (self);
	self->page_incoming = create_page_incoming (self);
	self->page_outgoing = create_page_outgoing (self);
	
	/* Add the notebook pages: */
	gtk_notebook_append_page (notebook, self->page_account_details, 
		gtk_label_new (_("mcen_ti_account_settings_account")));
	gtk_notebook_append_page (notebook, self->page_user_details, 
		gtk_label_new (_("mcen_ti_account_settings_userinfo")));
	gtk_notebook_append_page (notebook, self->page_incoming,
		gtk_label_new (_("mcen_ti_advsetup_retrieval")));
	gtk_notebook_append_page (notebook, self->page_outgoing,
		gtk_label_new (_("mcen_ti_advsetup_sending")));
		
	GtkDialog *dialog = GTK_DIALOG (self);
	gtk_container_add (GTK_CONTAINER (dialog->vbox), GTK_WIDGET (notebook));
	gtk_widget_show (GTK_WIDGET (notebook));
        
    /* Add the buttons: */
    gtk_dialog_add_button (GTK_DIALOG(self), GTK_STOCK_OK, GTK_RESPONSE_OK);
    gtk_dialog_add_button (GTK_DIALOG(self), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
    
    /* Connect to the dialog's response signal: */
    g_signal_connect_after (G_OBJECT (self), "response",
            G_CALLBACK (on_response), self);       
}

ModestAccountSettingsDialog*
modest_account_settings_dialog_new (void)
{
	return g_object_new (MODEST_TYPE_ACCOUNT_SETTINGS_DIALOG, NULL);
}

/** Update the UI with the stored account details, so they can be edited.
 * @account_name: Name of the account, which should contain incoming and outgoing server accounts.
 */
void modest_account_settings_dialog_set_account_name (ModestAccountSettingsDialog *dialog, const gchar* account_name)
{
	if (!account_name)
		return;
		
	/* Save the account name so we can refer to it if the user changes it: */
	if (dialog->original_account_name)
		g_free (dialog->original_account_name);
	dialog->original_account_name = g_strdup (account_name);
	
	/* Get the account data for this account name: */
	ModestAccountData *account_data = modest_account_mgr_get_account_data (dialog->account_manager, 
		account_name);
	if (!account_data) {
		g_printerr ("modest: failed to get account data for %s\n", account_name);
		return;
	}

	if (!(account_data->store_account)) {
		g_printerr ("modest: account has no stores: %s\n", account_name);
		return;
	}
		
	/* Show the account data in the widgets: */
	gtk_entry_set_text( GTK_ENTRY (dialog->entry_account_title),
		account_name ? account_name : "");
	gtk_entry_set_text( GTK_ENTRY (dialog->entry_user_name), 
		account_data->fullname ? account_data->fullname : "");
	gtk_entry_set_text( GTK_ENTRY (dialog->entry_user_email), 
		account_data->email ? account_data->email : "");
		
	const gboolean leave_on_server = modest_account_mgr_get_bool (dialog->account_manager, account_name,
		MODEST_ACCOUNT_LEAVE_ON_SERVER, FALSE /* not server account */);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->checkbox_leave_messages), leave_on_server);
		
		
	ModestServerAccountData *incoming_account = account_data->store_account;
	if (incoming_account) {
		gtk_entry_set_text( GTK_ENTRY (dialog->entry_user_username),
			incoming_account->username ? incoming_account->username : "");
		gtk_entry_set_text( GTK_ENTRY (dialog->entry_user_password), 
			incoming_account->password ? incoming_account->password : "");
			
		gtk_entry_set_text( GTK_ENTRY (dialog->entry_incomingserver), 
			incoming_account->hostname ? incoming_account->hostname : "");
			
		update_incoming_server_title (dialog, incoming_account->proto);
		update_incoming_server_security_choices (dialog, incoming_account->proto);
		
		/* TODO:
	gchar	         *uri;
	ModestProtocol    proto;
	gchar            *password;
	time_t		  last_updated;
	GSList           *options;
	*/
	
	}
	
	ModestServerAccountData *outgoing_account = account_data->transport_account;
	if (outgoing_account) {
		gtk_entry_set_text( GTK_ENTRY (dialog->entry_outgoingserver), 
			outgoing_account->hostname ? outgoing_account->hostname : "");
		
		/* TODO: Dim these if secure authentication is None, as per the UI spec: */
		gtk_entry_set_text( GTK_ENTRY (dialog->entry_outgoing_username), 
			outgoing_account->username ? outgoing_account->username : "");
		gtk_entry_set_text( GTK_ENTRY (dialog->entry_outgoing_password), 
			outgoing_account->password ? outgoing_account->password : "");
		
		/* How do we get the auth setting from the server account struct?: */
		easysetup_secureauth_combo_box_set_active_secureauth (
			EASYSETUP_SECUREAUTH_COMBO_BOX (dialog->combo_outgoing_auth), MODEST_PROTOCOL_AUTH_NONE);
		on_combo_outgoing_auth_changed (GTK_COMBO_BOX (dialog->combo_outgoing_auth), dialog);
		
		easysetup_serversecurity_combo_box_fill (
		EASYSETUP_SERVERSECURITY_COMBO_BOX (dialog->combo_outgoing_security), outgoing_account->proto);
		
		
		
		/* TODO: set port. */
	}
	
	/* TODO: account_data->display_name */
	/* account_data->is_enabled,  */
	/*account_data->is_default,  */

	/* account_data->store_account->proto */

	modest_account_mgr_free_account_data (dialog->account_manager, account_data);
}

static void
save_configuration (ModestAccountSettingsDialog *dialog)
{
	g_assert (dialog->original_account_name);
	
	const gchar* account_name = dialog->original_account_name;
		
	/* Set the account data from the widgets: */
	const gchar* user_name = gtk_entry_get_text (GTK_ENTRY (dialog->entry_user_name));
	modest_account_mgr_set_string (dialog->account_manager, account_name,
		MODEST_ACCOUNT_FULLNAME, user_name, FALSE /* not server account */);
		
	const gchar* emailaddress = gtk_entry_get_text (GTK_ENTRY (dialog->entry_user_email));
	modest_account_mgr_set_string (dialog->account_manager, account_name,
		MODEST_ACCOUNT_EMAIL, emailaddress, FALSE /* not server account */);
		
	/* TODO: Change name: */
	/* Possibly the account name may never change, but that should be hidden, 
	 * and the display name may change, defaulting to the account name.
	 */
	
	const gboolean leave_on_server = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->checkbox_leave_messages));
	modest_account_mgr_set_bool (dialog->account_manager, account_name,
		MODEST_ACCOUNT_LEAVE_ON_SERVER, leave_on_server, FALSE /* not server account */);
	
		
	/* TODO: How can we change the incoming and outgoing server account details? */
}

	
#if 0
static gboolean
on_before_next (GtkDialog *dialog, GtkWidget *current_page, GtkWidget *next_page)
{
	ModestAccountSettingsDialog *self = MODEST_ACCOUNT_SETTINGS_DIALOG (dialog);
	
	/* Do extra validation that couldn't be done for every key press,
	 * either because it was too slow,
	 * or because it requires interaction:
	 */
	if (current_page == self->page_account_details) {	
		/* Check that the title is not already in use: */
		const gchar* account_name = gtk_entry_get_text (GTK_ENTRY (self->entry_account_title));
		if ((!account_name) || (strlen(account_name) == 0))
			return FALSE;
			
		gboolean name_in_use = FALSE;
		name_in_use = modest_account_mgr_account_exists (self->account_manager,
			account_name, FALSE /*  server_account */);
		
		if (name_in_use) {
			/* Warn the user via a dialog: */
			show_error (GTK_WINDOW (self), _("mail_ib_account_name_already_existing."));
            
			return FALSE;
		}
	}
	else if (current_page == self->page_user_details) {
		/* Check that the email address is valud: */
		const gchar* email_address = gtk_entry_get_text (GTK_ENTRY (self->entry_user_email));
		if ((!email_address) || (strlen(email_address) == 0))
			return FALSE;
			
		if (!modest_text_utils_validate_email_address (email_address)) {
			/* Warn the user via a dialog: */
			show_error (GTK_WINDOW (self), _("mcen_ib_invalid_email"));
                                             
            /* Return focus to the email address entry: */
            gtk_widget_grab_focus (self->entry_user_email);
            
			return FALSE;
		}
		
		/* Make sure that the subsequent pages are appropriate for the provider choice. */
		create_subsequent_pages (self);
	}
	
	/* TODO: The UI Spec wants us to check that the servernames are valid, 
	 * but does not specify how.
	 */
	  
	if(next_page == self->page_incoming) {
		set_default_custom_servernames (self);
	}
	else if (next_page == self->page_outgoing) {
		set_default_custom_servernames (self);
	}
	
	/* If this is the last page, and this is a click on Finish, 
	 * then attempt to create the dialog.
	 */
	if(!next_page) /* This is NULL when this is a click on Finish. */
	{
		create_account (self);
	}
	
	
	return TRUE;
}
#endif

static gboolean entry_is_empty (GtkWidget *entry)
{
	if (!entry)
		return FALSE;
		
	const gchar* text = gtk_entry_get_text (GTK_ENTRY (entry));
	if ((!text) || (strlen(text) == 0))
		return TRUE;
	else
		return FALSE;
}

static void
enable_buttons (ModestAccountSettingsDialog *self)
{
	gboolean enable_ok = TRUE;
	
	/* The account details title is mandatory: */
	if (entry_is_empty(self->entry_account_title))
			enable_ok = FALSE;

	/* The user details username is mandatory: */
	if (entry_is_empty(self->entry_user_username))
		enable_ok = FALSE;
		
	/* The user details email address is mandatory: */
	if (enable_ok && entry_is_empty (self->entry_user_email))
		enable_ok = FALSE;

	/* The custom incoming server is mandatory: */
	if (entry_is_empty(self->entry_incomingserver))
		enable_ok = FALSE;
			
	/* Enable the buttons, 
	 * identifying them via their associated response codes:
	 */
	GtkDialog *dialog_base = GTK_DIALOG (self);
    gtk_dialog_set_response_sensitive (dialog_base,
                                       GTK_RESPONSE_OK,
                                       enable_ok);
}

static void
modest_account_settings_dialog_class_init (ModestAccountSettingsDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	g_type_class_add_private (klass, sizeof (ModestAccountSettingsDialogPrivate));


	object_class->get_property = modest_account_settings_dialog_get_property;
	object_class->set_property = modest_account_settings_dialog_set_property;
	object_class->dispose = modest_account_settings_dialog_dispose;
	object_class->finalize = modest_account_settings_dialog_finalize;
}
 
#if 0
static void
show_error (GtkWindow *parent_window, const gchar* text)
{
	GtkDialog *dialog = GTK_DIALOG (gtk_message_dialog_new (parent_window,
		(GtkDialogFlags)0,
		 GTK_MESSAGE_ERROR,
		 GTK_BUTTONS_OK,
		 text ));
		 
		 gtk_dialog_run (dialog);
		 gtk_widget_destroy (GTK_WIDGET (dialog));
}
#endif

#if 0
/** Attempt to create the account from the information that the user has entered.
 * @result: TRUE if the account was successfully created.
 */
gboolean
create_account (ModestAccountSettingsDialog *self)
{
	ModestAccountSettingsDialogPrivate *priv = ACCOUNT_SETTINGS_DIALOG_GET_PRIVATE (self);
	
	const gchar* account_name = gtk_entry_get_text (GTK_ENTRY (self->entry_account_title));

	/* Some checks: */
	if (!account_name)
		return FALSE;
		
	/* We should have checked for this already, 
	 * and changed that name accordingly, 
	 * but let's check again just in case:
	 */
	if (modest_account_mgr_account_exists (self->account_manager, account_name, FALSE)) 
		return FALSE;	
		
	/* username and password (for both incoming and outgoing): */
	const gchar* username = gtk_entry_get_text (GTK_ENTRY (self->entry_user_username));
	const gchar* password = gtk_entry_get_text (GTK_ENTRY (self->entry_user_password));
	
	/* Incoming server: */
	/* Note: We need something as default for the ModestProtocol values, 
	 * or modest_account_mgr_add_server_account will fail. */
	gchar* servername_incoming = NULL;
	ModestProtocol protocol_incoming = MODEST_PROTOCOL_STORE_POP;
	ModestProtocol protocol_security_incoming = MODEST_PROTOCOL_SECURITY_NONE;
	ModestProtocol protocol_authentication_incoming = MODEST_PROTOCOL_AUTH_NONE;
	
	
	/* Use custom pages because no preset was specified: */
	servername_incoming = g_strdup (gtk_entry_get_text (GTK_ENTRY (self->entry_incomingserver) ));
	
	protocol_incoming = easysetup_servertype_combo_box_get_active_servertype (
	EASYSETUP_SERVERTYPE_COMBO_BOX (self->combo_incoming_servertype));
	
	protocol_security_incoming = easysetup_serversecurity_combo_box_get_active_serversecurity (
	EASYSETUP_SERVERSECURITY_COMBO_BOX (self->combo_incoming_security));
	
	protocol_authentication_incoming = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (self->checkbox_incoming_auth)) 
		? MODEST_PROTOCOL_AUTH_PASSWORD
		: MODEST_PROTOCOL_AUTH_NONE;
		

	
	/* First we add the 2 server accounts, and then we add the account that uses them.
	 * If we don't do it in this order then we will experience a crash. */
	 
	/* Add a (incoming) server account, to be used by the account: */
	gchar *store_name = g_strconcat (account_name, "_store", NULL);
	gboolean created = modest_account_mgr_add_server_account (self->account_manager,
		store_name,
		servername_incoming,
		username, password,
		protocol_incoming,
		protocol_security_incoming,
		protocol_authentication_incoming);		
		
	g_free (servername_incoming);
	
	if (!created) {
		/* TODO: Provide a Logical ID for the text: */
		show_error (GTK_WINDOW (self), _("An error occurred while creating the incoming account."));
		return FALSE;	
	}
	
	/* Sanity check: */
	/* There must be at least one account now: */
	GSList *account_names = modest_account_mgr_account_names (self->account_manager);
	if(account_names != NULL)
	{
		g_warning ("modest_account_mgr_account_names() returned NULL after adding an account.");
	}
	g_slist_free (account_names);
	
	
	/* Outgoing server: */
	gchar* servername_outgoing = NULL;
	ModestProtocol protocol_outgoing = MODEST_PROTOCOL_STORE_POP;
	ModestProtocol protocol_security_outgoing = MODEST_PROTOCOL_SECURITY_NONE;
	ModestProtocol protocol_authentication_outgoing = MODEST_PROTOCOL_AUTH_NONE;
	
	
	/* Use custom pages because no preset was specified: */
	servername_outgoing = g_strdup (gtk_entry_get_text (GTK_ENTRY (self->entry_outgoingserver) ));
	
	protocol_outgoing = MODEST_PROTOCOL_TRANSPORT_SMTP; /* It's always SMTP for outgoing. */

	protocol_security_outgoing = easysetup_serversecurity_combo_box_get_active_serversecurity (
		EASYSETUP_SERVERSECURITY_COMBO_BOX (self->combo_outgoing_security));
	
	protocol_authentication_outgoing = easysetup_secureauth_combo_box_get_active_secureauth (
		EASYSETUP_SECUREAUTH_COMBO_BOX (self->combo_outgoing_auth));
	
	/* TODO: 
	gboolean specific = gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON (self->checkbox_outgoing_smtp_specific));
	*/
		
	    
	/* Add a (outgoing) server account to be used by the account: */
	gchar *transport_name = g_strconcat (account_name, "_transport", NULL); /* What is this good for? */
	created = modest_account_mgr_add_server_account (self->account_manager,
		transport_name,
		servername_outgoing,
		username, password,
		protocol_outgoing,
		protocol_security_outgoing,
		protocol_authentication_outgoing);
		
	g_free (servername_outgoing);
		
	if (!created) {
		/* TODO: Provide a Logical ID for the text: */
		show_error (GTK_WINDOW (self), _("An error occurred while creating the outgoing account."));
		return FALSE;	
	}
	
	
	/* Create the account, which will contain the two "server accounts": */
 	created = modest_account_mgr_add_account (self->account_manager, account_name, 
 		store_name, /* The name of our POP/IMAP server account. */
		transport_name /* The name of our SMTP server account. */);
	g_free (store_name);
	g_free (transport_name);
	
	if (!created) {
		/* TODO: Provide a Logical ID for the text: */
		show_error (GTK_WINDOW (self), _("An error occurred while creating the account."));
		return FALSE;	
	}
	
	return FALSE;
}
#endif


