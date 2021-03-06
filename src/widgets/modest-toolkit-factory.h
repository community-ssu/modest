#ifndef                                         __MODEST_TOOLKIT_FACTORY_H__
#define                                         __MODEST_TOOLKIT_FACTORY_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <modest-presets.h>
#include <widgets/modest-security-options-view.h>
#include <modest-pair.h>
#include <tny-header.h>
#include <tny-folder.h>

G_BEGIN_DECLS

#define                                         MODEST_TYPE_TOOLKIT_FACTORY \
                                                (modest_toolkit_factory_get_type())

#define                                         MODEST_TOOLKIT_FACTORY(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                MODEST_TYPE_TOOLKIT_FACTORY, ModestToolkitFactory))

#define                                         MODEST_TOOLKIT_FACTORY_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                                MODEST_TYPE_TOOLKIT_FACTORY, ModestToolkitFactory))

#define                                         MODEST_IS_TOOLKIT_FACTORY(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MODEST_TYPE_TOOLKIT_FACTORY))

#define                                         MODEST_IS_TOOLKIT_FACTORY_CLASS(klass) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((klass), MODEST_TYPE_TOOLKIT_FACTORY))

#define                                         MODEST_TOOLKIT_FACTORY_GET_CLASS(obj) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                                MODEST_TYPE_TOOLKIT_FACTORY, ModestToolkitFactoryClass))

typedef struct                                  _ModestToolkitFactory ModestToolkitFactory;

typedef struct                                  _ModestToolkitFactoryClass ModestToolkitFactoryClass;

struct                                          _ModestToolkitFactoryClass
{
	GObjectClass parent_class;

	GtkWidget * (*create_scrollable) (ModestToolkitFactory *self);
	GtkWidget * (*create_check_button) (ModestToolkitFactory *self, const gchar *label);
	GtkWidget * (*create_check_menu) (ModestToolkitFactory *self, const gchar *label);
	GtkWidget * (*create_isearch_toolbar) (ModestToolkitFactory *self, const gchar *label);
	GtkWidget * (*create_entry) (ModestToolkitFactory *self);
	GtkWidget * (*create_number_entry) (ModestToolkitFactory *self, gint min, gint max);
	GtkWidget * (*create_file_chooser_dialog) (ModestToolkitFactory *self, const gchar *title,
						   GtkWindow *parent, GtkFileChooserAction action);
	GtkWidget * (*create_country_selector) (ModestToolkitFactory *self);
	GtkWidget * (*create_provider_selector) (ModestToolkitFactory *self);
	GtkWidget * (*create_servertype_selector) (ModestToolkitFactory *self, gboolean filter_providers);
	GtkWidget * (*create_serversecurity_selector) (ModestToolkitFactory *self);
	GtkWidget * (*create_secureauth_selector) (ModestToolkitFactory *self);
	GtkWidget * (*create_security_options_view) (ModestToolkitFactory *self, ModestSecurityOptionsType type,
						     gboolean full, GtkSizeGroup *title_size_group, GtkSizeGroup *value_size_group);
	GtkWidget * (*create_text_view) (ModestToolkitFactory *self);
	GtkWidget * (*create_selector) (ModestToolkitFactory *self,
					ModestPairList *pairs, GEqualFunc id_equal_func, gboolean center);
	GtkWidget * (*create_details_dialog_with_header) (ModestToolkitFactory *self, GtkWindow *parent,
							  TnyHeader *header, gboolean get_size);
	GtkWidget * (*create_details_dialog_with_folder) (ModestToolkitFactory *self, GtkWindow *parent,
							  TnyFolder *folder);
};

struct                                          _ModestToolkitFactory
{
    GObject parent;
};


GType
modest_toolkit_factory_get_type                       (void) G_GNUC_CONST;

ModestToolkitFactory *
modest_toolkit_factory_get_instance                            (void);

GtkWidget *
modest_toolkit_factory_create_scrollable              (ModestToolkitFactory *self);

GtkWidget *
modest_toolkit_factory_create_check_button (ModestToolkitFactory *self, const gchar *label);

GtkWidget *
modest_toolkit_factory_create_check_menu (ModestToolkitFactory *self, const gchar *label);

GtkWidget *
modest_toolkit_factory_create_isearch_toolbar (ModestToolkitFactory *self, const gchar *label);

GtkWidget *
modest_toolkit_factory_create_entry (ModestToolkitFactory *self);

GtkWidget *
modest_toolkit_factory_create_number_entry (ModestToolkitFactory *self, gint min, gint max);

GtkWidget *
modest_toolkit_factory_create_file_chooser_dialog (ModestToolkitFactory *self, const gchar *title,
						   GtkWindow *parent, GtkFileChooserAction action);

GtkWidget *
modest_toolkit_factory_create_country_selector (ModestToolkitFactory *self);

GtkWidget *
modest_toolkit_factory_create_provider_selector (ModestToolkitFactory *self);

GtkWidget *
modest_toolkit_factory_create_servertype_selector (ModestToolkitFactory *self, gboolean filter_providers);

GtkWidget *
modest_toolkit_factory_create_serversecurity_selector (ModestToolkitFactory *self);

GtkWidget *
modest_toolkit_factory_create_secureauth_selector (ModestToolkitFactory *self);

GtkWidget *
modest_toolkit_factory_create_security_options_view (ModestToolkitFactory *self, ModestSecurityOptionsType type,
						     gboolean full, GtkSizeGroup *title_size_group, GtkSizeGroup *value_size_group);

GtkWidget *
modest_toolkit_factory_create_text_view (ModestToolkitFactory *self);

GtkWidget *
modest_toolkit_factory_create_selector (ModestToolkitFactory *self,
					ModestPairList *pairs, GEqualFunc id_equal_func, gboolean center);

GtkWidget *
modest_toolkit_factory_create_details_dialog_with_header (ModestToolkitFactory *self, GtkWindow *parent,
							  TnyHeader *header, gboolean get_size);

GtkWidget *
modest_toolkit_factory_create_details_dialog_with_folder (ModestToolkitFactory *self, GtkWindow *parent,
							  TnyFolder *folder);

gboolean
modest_togglable_get_active (GtkWidget *widget);

void
modest_togglable_set_active (GtkWidget *widget, gboolean active);

gboolean
modest_is_togglable (GtkWidget *widget);

void
modest_entry_set_text (GtkWidget *widget, const gchar *text);

const gchar *
modest_entry_get_text (GtkWidget *widget);

void
modest_entry_set_hint (GtkWidget *widget, const gchar *hint);

gboolean 
modest_is_entry (GtkWidget *widget);

gint
modest_number_entry_get_value (GtkWidget *widget);

void
modest_number_entry_set_value (GtkWidget *widget, gint value);

gboolean
modest_number_entry_is_valid (GtkWidget *widget);

gboolean
modest_is_number_entry (GtkWidget *widget);

gint
modest_country_selector_get_active_country_mcc (GtkWidget *widget);

void
modest_country_selector_load_data (GtkWidget *widget);

gboolean
modest_country_selector_set_active_country_locale (GtkWidget *widget);

typedef enum {
	MODEST_PROVIDER_SELECTOR_ID_PROVIDER,
	MODEST_PROVIDER_SELECTOR_ID_OTHER,
	MODEST_PROVIDER_SELECTOR_ID_PLUGIN_PROTOCOL
} ModestProviderSelectorIdType;


void
modest_provider_selector_fill (GtkWidget *widget, ModestPresets *presets, gint mcc);

void
modest_provider_selector_refresh (GtkWidget *widget);

gchar *
modest_provider_selector_get_active_provider_id (GtkWidget *widget);

gchar *
modest_provider_selector_get_active_provider_label (GtkWidget *widget);

ModestProviderSelectorIdType
modest_provider_selector_get_active_id_type (GtkWidget *widget);

void
modest_provider_selector_set_others_provider (GtkWidget *self);

ModestProtocolType
modest_servertype_selector_get_active_servertype (GtkWidget *self);

void
modest_servertype_selector_set_active_servertype (GtkWidget *self, ModestProtocolType protocol_type_id);

void modest_serversecurity_selector_fill (GtkWidget *combobox, ModestProtocolType protocol);

ModestProtocolType modest_serversecurity_selector_get_active_serversecurity (GtkWidget *combobox);

gboolean modest_serversecurity_selector_set_active_serversecurity (GtkWidget *combobox,
								   ModestProtocolType serversecurity);

gint modest_serversecurity_selector_get_active_serversecurity_port (GtkWidget *combobox);

ModestProtocolType
modest_secureauth_selector_get_active_secureauth (GtkWidget *widget);

gboolean
modest_secureauth_selector_set_active_secureauth (GtkWidget *widget,
						  ModestProtocolType secureauth);

gboolean
modest_is_secureauth_selector (GtkWidget *widget);

GtkTextBuffer *
modest_text_view_get_buffer (GtkWidget *widget);

gboolean 
modest_is_text_view (GtkWidget *widget);

void         modest_selector_set_pair_list (GtkWidget *widget, 
					    ModestPairList *pairs);
gpointer   modest_selector_get_active_id  (GtkWidget *self);
void       modest_selector_set_active_id (GtkWidget *self, gpointer id);
const gchar* modest_selector_get_active_display_name  (GtkWidget *self);
void       modest_selector_set_value_max_chars (GtkWidget *self, gint value_max_width_chars);
gint       modest_selector_get_value_max_chars (GtkWidget *self);


#ifndef MODEST_TOOLKIT_HILDON2
#define USE_PROVIDER_COMBOBOX
#define USE_SERVERTYPE_COMBOBOX
#define USE_GTK_SPIN_BUTTON
#endif

#ifndef USE_GTK_SPIN_BUTTON
#define MODEST_NUMBER_ENTRY_SUPPORT_VALID_CHANGED
#endif

#ifndef USE_PROVIDER_COMBOBOX
#include <hildon/hildon.h>
#endif
#ifndef USE_SERVERTYPE_COMBOBOX
#include <hildon/hildon.h>
#endif

G_END_DECLS

#endif /* __MODEST_WP_TEXT_VIEW_H__ */
