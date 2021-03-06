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

#ifndef __MODEST_WINDOW_H__
#define __MODEST_WINDOW_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /*HAVE_CONFIG_H*/

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/* 
 * admittedly, the ifdefs for gtk and maemo are rather ugly; still
 * this way is probably the easiest to maintain
 */
#ifdef MODEST_TOOLKIT_GTK
typedef GtkVBox      ModestWindowParent;
typedef GtkVBoxClass ModestWindowParentClass;
#else
#ifdef MODEST_TOOLKIT_HILDON2
#include <hildon/hildon-stackable-window.h>
typedef HildonStackableWindow      ModestWindowParent;
typedef HildonStackableWindowClass ModestWindowParentClass;
#else
#include <hildon/hildon-window.h>
typedef HildonWindow      ModestWindowParent;
typedef HildonWindowClass ModestWindowParentClass;
#endif

#ifndef GTK_STOCK_FULLSCREEN
#define GTK_STOCK_FULLSCREEN ""
#endif /*GTK_STOCK_FULLSCREEN*/

#endif /*!MODEST_TOOLKIT_GTK */

/* Dimmed state variables */
typedef struct _DimmedState {	
	guint    n_selected;
	guint    already_opened_msg;
	gboolean any_marked_as_deleted;
	gboolean all_marked_as_deleted;
	gboolean any_marked_as_seen;
	gboolean all_marked_as_seen;
	gboolean any_marked_as_cached;
	gboolean all_marked_as_cached;
	gboolean any_has_attachments;
	gboolean all_has_attachments;
	gboolean sent_in_progress;
	gboolean all_selected;
} DimmedState;

/* convenience macros */
#define MODEST_TYPE_WINDOW             (modest_window_get_type())
#define MODEST_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MODEST_TYPE_WINDOW,ModestWindow))
#define MODEST_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MODEST_TYPE_WINDOW,GObject))
#define MODEST_IS_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MODEST_TYPE_WINDOW))
#define MODEST_IS_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MODEST_TYPE_WINDOW))
#define MODEST_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MODEST_TYPE_WINDOW,ModestWindowClass))

typedef struct _ModestWindow      ModestWindow;
typedef struct _ModestWindowClass ModestWindowClass;

typedef void (*ModestWindowMenuCallback) (GObject *control, gpointer userdata);
typedef gboolean (*ModestDimmingCallback) (ModestWindow *self, gpointer user_data);

#define MODEST_WINDOW_MENU_CALLBACK(x) ((ModestWindowMenuCallback) (x))
#define MODEST_DIMMING_CALLBACK(x) ((ModestDimmingCallback) (x))
 
struct _ModestWindow {
	 ModestWindowParent parent;
};

struct _ModestWindowClass {
	ModestWindowParentClass parent_class;

	/* virtual methods */
	void (*set_zoom_func) (ModestWindow *self, gdouble zoom);
	gdouble (*get_zoom_func) (ModestWindow *self);
	gboolean (*zoom_plus_func) (ModestWindow *self);
	gboolean (*zoom_minus_func) (ModestWindow *self);
	void (*show_toolbar_func) (ModestWindow *self, gboolean show_toolbar);
	void (*save_state_func) (ModestWindow *self);
	void (*disconnect_signals_func) (ModestWindow *self);
	void (*show_progress_func) (ModestWindow *self, gboolean show);
	void (*add_toolbar_func) (ModestWindow *self, GtkToolbar *toolbar);
	void (*set_title_func) (ModestWindow *self, const gchar *title);
	void (*add_to_menu_func) (ModestWindow *self,
				  const gchar *label,
				  const gchar *accelerator,
				  ModestWindowMenuCallback callback,
				  ModestDimmingCallback dimming_callback);
	void (*add_item_to_menu_func) (ModestWindow *self,
				       GtkWidget *widget,
				       ModestDimmingCallback dimming_callback);
	void (*pack_toolbar_func) (ModestWindow *self, GtkPackType pack_type, GtkWidget *toolbar);
	void (*edit_mode_changed) (ModestWindow *self, gint edit_mode, gboolean enabled);
};


/**
 * modest_window_get_type:
 *
 * get the #GType for #ModestWindow
 * 
 * Returns: the type
 */	
GType        modest_window_get_type    (void) G_GNUC_CONST;


/**
 * modest_window_get_active_account:
 * @self: a modest window instance
 * 
 * get the name of the active account
 * 
 * Returns: the active account name as a constant string
 */	
const gchar* modest_window_get_active_account (ModestWindow *self);



/**
 * modest_window_set_active_account:
 * @self: a modest window instance
 * @active_account: a new active account name for this window
 * 
 * set the active account for this window.
 * NOTE: this must be a valid, non-pseudo account.
 * 
 */	
void modest_window_set_active_account (ModestWindow *self, const gchar *active_account);

/**
 * modest_window_get_active_mailbox:
 * @self: a modest window instance
 * 
 * get the name of the active mailbox
 * 
 * Returns: the active mailbox as a constant string
 */	
const gchar* modest_window_get_active_mailbox (ModestWindow *self);



/**
 * modest_window_set_active_account:
 * @self: a modest window instance
 * @active_mailbox: a new active mailbox name for this window
 * 
 * set the active mailbox for this window.
 * 
 */	
void modest_window_set_active_mailbox (ModestWindow *self, const gchar *active_mailbox);

/**
 * modest_window_set_zoom:
 * @window: a #ModestWindow instance
 * @zoom: the zoom level (1.0 is no zoom)
 *
 * sets the zoom level of the window
 */
void            modest_window_set_zoom    (ModestWindow *window,
					   gdouble value);

/**
 * modest_window_get_zoom:
 * @window: a #ModestWindow instance
 *
 * gets the zoom of the window
 *
 * Returns: the current zoom value (1.0 is no zoom)
 */
gdouble         modest_window_get_zoom    (ModestWindow *window);

/**
 * modest_window_zoom_plus:
 * @window: a #ModestWindow
 *
 * increases one level the zoom.
 *
 * Returns: %TRUE if successful, %FALSE if increasing zoom is not available
 */
gboolean modest_window_zoom_plus (ModestWindow *window);

/**
 * modest_window_zoom_minus:
 * @window: a #ModestWindow
 *
 * decreases one level the zoom.
 *
 * Returns: %TRUE if successful, %FALSE if increasing zoom is not available
 */
gboolean modest_window_zoom_minus (ModestWindow *window);


/**
 * modest_window_show_toolbar:
 * @window: 
 * @view_toolbar: whether or not the toolbar should be shown
 * 
 * shows/hides the window toolbar
 **/
void     modest_window_show_toolbar (ModestWindow *window, 
				     gboolean show_toolbar);
				     
/**
 * modest_window_save_state:
 * @window: 
 * 
 * Ask the window to save its settings for loading again later.
 * This actually invokes the save_setting_func vfunc, which 
 * derived windows should implement.
 **/
void     modest_window_save_state (ModestWindow *window);


/**
 * modest_window_set_dimming_state:
 * @window: a #ModestWindow instance object
 * @state: the #DimmedState state at specific time 
 * 
 * Set basic dimming variables from selected headers at
 * specific moment.
 **/
void
modest_window_set_dimming_state (ModestWindow *window,
				 DimmedState *state);

/**
 * modest_window_set_dimming_state:
 * @window: a #ModestWindow instance object
 * 
 * Set basic dimming variables from selected headers at
 * specific moment.
 * 
 * @Returns: a  #DimmedState state saved previously. 
 **/
const DimmedState *
modest_window_get_dimming_state (ModestWindow *window);
				

/**
 * modest_window_get_action:
 * @window: a #ModestWindow instance object
 * @action_path: the full path of required action.
 * 
 * Get an action from ui manager, using @action_path parameter,
 * which represent the full path to the required action into UIManager 
 * xml definition.
 **/
GtkAction * modest_window_get_action (ModestWindow *window, const gchar *action_path); 

/**
 * modest_window_get_action_widget:
 * @window: a #ModestWindow instance object
 * @action_path: the full path of required action.
 * 
 * Get action widget from ui manager, using @action_path parameter,
 * which represent the full path to the required action into UIManager 
 * xml definition.
 **/
GtkWidget *modest_window_get_action_widget (ModestWindow *window, 
					    const gchar *action_path);

/**
 * modest_window_check_dimming_rules:
 * @self: a #ModestWindow instance object
 * 
 * Calls UI Dimming Manager of @self window to check all dimming rules.
 * 
 **/
void modest_window_check_dimming_rules (ModestWindow *self);

/**
 * modest_window_check_dimming_rules:
 * @self: a #ModestWindow instance object
 * @group: a #ModestWindow instance object
 * 
 * Calls UI Dimming Manager of @self window to check @group_name specific
 * dimming rules.
 * 
 **/
void modest_window_check_dimming_rules_group (ModestWindow *self,
					      const gchar *group_name);


/**
 * modest_window_enable_dimming:
 * @self: a #ModestWindow instance object
 * 
 * Enables UI dimming rules checking.
 *
 **/
void modest_window_enable_dimming (ModestWindow *self);


/**
 * modest_window_disable_dimming:
 * @self: a #ModestWindow instance object
 * 
 * Disables UI dimming rules checking.
 *
 **/
void modest_window_disable_dimming (ModestWindow *self);


void modest_window_disconnect_signals (ModestWindow *self);

void modest_window_show_progress (ModestWindow *self, gboolean show_progress);

void modest_window_add_toolbar (ModestWindow *self, GtkToolbar *toolbar);

void modest_window_set_title (ModestWindow *self, const gchar *title);

/**
 * modest_window_add_to_menu:
 * @self: a #ModestWindow
 * @label: the label of the button added to menu
 * @callback: a #ModestWindowMenuCallback
 * @dimming_callback: a #ModestDimmingCallback or %NULL for no dimming rule
 *
 * creates a menu item in the window @self, with label @label.
 * It will call @callback, and will configure @dimming_callback for
 * dimming.
 */
void modest_window_add_to_menu (ModestWindow *self,
				const gchar *label,
				const gchar *accelerator,
				ModestWindowMenuCallback callback,
				ModestDimmingCallback dimming_callback);

/**
 * modest_window_add_item_to_menu:
 * @self: a #ModestWindow
 * @item: a #GtkWidget (a #GtkButton in hildon, a #GtkMenuItem in gtk)
 * @dimming_callback: a #ModestDimmingCallback or %NULL for no dimming rule
 *
 * adds a previously configured item @item to the window menu.
 * It will configure @dimming_callback for dimming. User should previously
 * attach the proper signal handler for @button.
 */
void 
modest_window_add_item_to_menu (ModestWindow *self,
				GtkWidget *item,
				ModestDimmingCallback dimming_callback);

/**
 * modest_window_pack_toolbar:
 * @self: a #ModestWindow
 * @pack_type: a #GtkPackType
 * @toolbar: a toolbar widget
 *
 * packs a toolbar (widget @toolbar) in @self with @pack_type
 */
void modest_window_pack_toolbar (ModestWindow *self,
				 GtkPackType pack_type,
				 GtkWidget *toolbar);


G_END_DECLS

#endif /* __MODEST_WINDOW_H__ */
