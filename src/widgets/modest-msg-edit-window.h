/* Copyright (c) 2006,2007 Nokia Corporation
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

#ifndef __MODEST_MSG_EDIT_WINDOW_H__
#define __MODEST_MSG_EDIT_WINDOW_H__

#include <tny-msg.h>
#include <widgets/modest-window.h>

G_BEGIN_DECLS

/* convenience macros */
#define MODEST_TYPE_MSG_EDIT_WINDOW             (modest_msg_edit_window_get_type())
#define MODEST_MSG_EDIT_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MODEST_TYPE_MSG_EDIT_WINDOW,ModestMsgEditWindow))
#define MODEST_MSG_EDIT_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MODEST_TYPE_MSG_EDIT_WINDOW,ModestWindow))
#define MODEST_IS_MSG_EDIT_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MODEST_TYPE_MSG_EDIT_WINDOW))
#define MODEST_IS_MSG_EDIT_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MODEST_TYPE_MSG_EDIT_WINDOW))
#define MODEST_MSG_EDIT_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MODEST_TYPE_MSG_EDIT_WINDOW,ModestMsgEditWindowClass))

typedef struct _ModestMsgEditWindow      ModestMsgEditWindow;
typedef struct _ModestMsgEditWindowClass ModestMsgEditWindowClass;

struct _ModestMsgEditWindow {
	 ModestWindow parent;
	/* insert public members, if any */
};

struct _ModestMsgEditWindowClass {
	ModestWindowClass parent_class;
	/* insert signal callback declarations, eg. */
	/* void (* my_event) (ModestMsgEditWindow* obj); */
};

typedef enum  {
	MODEST_EDIT_TYPE_NEW,
	MODEST_EDIT_TYPE_REPLY,
	MODEST_EDIT_TYPE_FORWARD,
	
	MODEST_EDIT_TYPE_NUM
} ModestEditType;

typedef enum {
	MODEST_MSG_EDIT_FORMAT_TEXT,
	MODEST_MSG_EDIT_FORMAT_HTML
} ModestMsgEditFormat;

/** Get these with modest_msg_edit_window_get_msg_data() 
 * and free them with modest_msg_edit_window_free_msg_data().
 */
typedef struct  {
	gchar *from, *to, *cc, *bcc, *subject, *plain_body, *html_body;
	GList *attachments;
	TnyHeaderFlags priority_flags;
	TnyMsg *draft_msg;
	gchar *account_name;
} MsgData;

typedef struct {
	gboolean bold;
	gboolean italics;
	gboolean bullet;
	GdkColor color;
	const gchar *font_family;
	gint font_size;
	GtkJustification justification;
} ModestMsgEditFormatState;


/**
 * modest_msg_edit_window_get_type:
 * 
 * get the GType for the #ModestMsgEditWindow class
 *
 * Returns: a GType for #ModestMsgEditWindow
 */
GType        modest_msg_edit_window_get_type    (void) G_GNUC_CONST;


/**
 * modest_msg_edit_window_new:
 * #msg: a #TnyMsg instance
 * #account_name: the account this message applies to
 * 
 * instantiates a new #ModestMsgEditWindow widget
 *
 * Returns: a new #ModestMsgEditWindow, or NULL in case of error
 */
ModestWindow*   modest_msg_edit_window_new         (TnyMsg *msg, const gchar *account_name);


/**
 * modest_msg_edit_window_get_msg_data:
 * @self: a #ModestMsgEditWindow
 * 
 * gets the message data already present in the edit message
 * window. The message data must be freed with
 * modest_msg_edit_window_free_msg_data
 * 
 * Returns: the message data
 **/
MsgData *               modest_msg_edit_window_get_msg_data          (ModestMsgEditWindow *self);

/**
 * modest_msg_edit_window_free_msg_data:
 * @self: a #ModestMsgEditWindow
 * @data: 
 * 
 * frees the message data passed as argument
 **/
void                    modest_msg_edit_window_free_msg_data         (ModestMsgEditWindow *self,
								      MsgData *data);

/**
 * modest_msg_edit_window_get_format:
 * @self: a #ModestMsgEditWindow
 *
 * obtains the format type of the message body.
 *
 * Returns: a #ModestMsgEditFormat
 **/
ModestMsgEditFormat     modest_msg_edit_window_get_format            (ModestMsgEditWindow *self);

/**
 * modest_msg_edit_window_set_format:
 * @self: a #ModestMsgEditWindow
 * @format: a #ModestMsgEditFormat
 *
 * set the @format of the edit window message body.
 **/
void                    modest_msg_edit_window_set_format            (ModestMsgEditWindow *self,
								      ModestMsgEditFormat format);

/**
 * modest_msg_edit_window_get_format_state:
 * @self: a #ModestMsgEditWindow
 *
 * get the current format state (the format attributes the text user inserts
 * will get).
 *
 * Returns: a #ModestMsgEditFormatState structure that should be freed with g_free().
 **/
ModestMsgEditFormatState *modest_msg_edit_window_get_format_state    (ModestMsgEditWindow *self);

/**
 * modest_msg_edit_window_set_format_state:
 * @self: a #ModestMsgEditWindow
 * @format_state: a #ModestMsgEditWindowFormatState
 *
 * sets a new format state (the format attributes the text user inserts 
 * will get).
 **/
void                    modest_msg_edit_window_set_format_state      (ModestMsgEditWindow *self,
								      const ModestMsgEditFormatState *format_state);

/**
 * modest_msg_edit_window_select_color:
 * @self: a #ModestMsgEditWindow
 *
 * show color selection dialog and update text color
 */
void                    modest_msg_edit_window_select_color          (ModestMsgEditWindow *window);

/**
 * modest_msg_edit_window_select_background_color:
 * @self: a #ModestMsgEditWindow
 *
 * show color selection dialog and update background color
 */
void                    modest_msg_edit_window_select_background_color          (ModestMsgEditWindow *window);

/**
 * modest_msg_edit_window_insert_image:
 * @self: a #ModestMsgEditWindow
 *
 * show a file selection dialog to insert an image
 */
void                    modest_msg_edit_window_insert_image          (ModestMsgEditWindow *window);

/**
 * modest_msg_edit_window_attach_file:
 * @self: a #ModestMsgEditWindow
 *
 * show a file selection dialog to attach a file
 */
void                    modest_msg_edit_window_attach_file           (ModestMsgEditWindow *window);

/**
 * modest_msg_edit_window_attach_file_noninteractive:
 * @self: a #ModestMsgEditWindow
 *
 * attach a file to a MsgEditWindow non interactively, 
 * without file dialog. This is needed by dbus callbacks.
 */
void                    modest_msg_edit_window_attach_file_noninteractive           (ModestMsgEditWindow *window, gchar *filename);

/**
 * modest_msg_edit_window_remove_attachments:
 * @self: a #ModestMsgEditWindow
 * @att_list: a #GList of #TnyMimePart
 *
 * remove attachments in @att_list, with a confirmation dialog
 */
void                    modest_msg_edit_window_remove_attachments    (ModestMsgEditWindow *window, 
								      GList *att_list);

/**
 * modest_msg_edit_window_show_cc:
 * @window: a #ModestMsgEditWindow
 * @show: a #gboolean
 *
 * Set the CC field as visible (or not visible) depending on @show
 */
void                    modest_msg_edit_window_show_cc               (ModestMsgEditWindow *window, gboolean show);

/**
 * modest_msg_edit_window_show_bcc:
 * @window: a #ModestMsgEditWindow
 * @show: a #gboolean
 *
 * Set the BCC field as visible (or not visible) depending on @show
 */
void                    modest_msg_edit_window_show_bcc               (ModestMsgEditWindow *window, gboolean show);

/**
 * modest_msg_edit_window_toggle_fullscreen:
 * @window: a #ModestMsgEditWindow
 *
 * toggles the current fullscreen status
 */
void            modest_msg_edit_window_toggle_fullscreen (ModestMsgEditWindow *window);

/**
 * modest_msg_edit_window_set_priority_flags:
 * @window: a #ModestMsgEditWindow
 * @priority_flags: a #TnyHeaderFlags with priority information
 *
 * Updates the icon and priority flag to send.
 */
void            modest_msg_edit_window_set_priority_flags (ModestMsgEditWindow *window,
							   TnyHeaderFlags priority_flags);

/**
 * modest_msg_edit_window_set_file_format:
 * @window: a #ModestMsgEditWindow
 * @file_format: %MODEST_FILE_FORMAT_PLAIN_TEXT or %MODEST_FILE_FORMAT_FORMATTED_TEXT
 *
 * Changes the current file format.
 */
void            modest_msg_edit_window_set_file_format (ModestMsgEditWindow *window,
							gint file_format);

/**
 * modest_msg_edit_window_select_font:
 * @window: a #ModestMsgEditWindow
 *
 * Show the dialog to select the editor font and update the
 * used font in the editor.
 */
void            modest_msg_edit_window_select_font        (ModestMsgEditWindow *window);

/**
 * modest_msg_edit_window_undo:
 * @window: a #ModestMsgEditWindow
 *
 * Undoes the last operation.
 */
void            modest_msg_edit_window_undo               (ModestMsgEditWindow *window);

/**
 * modest_msg_edit_window_select_contacts:
 * @window: a #ModestMsgEditWindow
 *
 * Shows the dialog to add contacts to the currently focused recipient list,
 * or to To: recipient if no recipient list is focused.
 */
void            modest_msg_edit_window_select_contacts    (ModestMsgEditWindow *window);

/**
 * modest_msg_edit_window_check_names:
 * @window: a #ModestMsgEditWindow
 *
 * Validates all the recipients, and shows (if required) dialogs for adding contacts
 * or fixing problems in specific fields.
 *
 * Returns: %TRUE if all fields were validated, %FALSE otherwise
 */
gboolean        modest_msg_edit_window_check_names    (ModestMsgEditWindow *window);

/**
 * modest_msg_edit_window_toggle_find_toolbar:
 * @window: a #ModestMsgEditWindow
 * @show: a #gboolean
 *
 * Shows/Hides the find toolbar
 */
void            modest_msg_edit_window_toggle_find_toolbar (ModestMsgEditWindow *window,
							    gboolean show);
G_END_DECLS

#endif /* __MODEST_MSG_EDIT_WINDOW_H__ */
