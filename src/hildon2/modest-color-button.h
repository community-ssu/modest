/* Copyright (c) 2009 Nokia Corporation
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

#ifndef __MODEST_COLOR_BUTTON_H__
#define __MODEST_COLOR_BUTTON_H__

#include <hildon/hildon-color-button.h>

G_BEGIN_DECLS

/* convenience macros */
#define MODEST_TYPE_COLOR_BUTTON             (modest_color_button_get_type())
#define MODEST_COLOR_BUTTON(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MODEST_TYPE_COLOR_BUTTON,ModestColorButton))
#define MODEST_COLOR_BUTTON_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MODEST_TYPE_COLOR_BUTTON,ModestWindow))

#define MODEST_IS_COLOR_BUTTON(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MODEST_TYPE_COLOR_BUTTON))
#define MODEST_IS_COLOR_BUTTON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MODEST_TYPE_COLOR_BUTTON))
#define MODEST_COLOR_BUTTON_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MODEST_TYPE_COLOR_BUTTON,ModestColorButtonClass))

typedef struct _ModestColorButton      ModestColorButton;
typedef struct _ModestColorButtonClass ModestColorButtonClass;

struct _ModestColorButton {
	HildonColorButton parent;
};

struct _ModestColorButtonClass {
	HildonColorButtonClass parent_class;
};

/**
 * modest_color_button_get_type:
 *
 * get the GType for the ModestColorButton class
 *
 * Returns: a GType for ModestColorButton
 */
GType modest_color_button_get_type (void) G_GNUC_CONST;


/**
 * modest_color_button_new:
 *
 * instantiates a new ModestColorButton widget
 *
 * Returns: a new ModestColorButton, or NULL in case of error
 */
GtkWidget* modest_color_button_new ();

G_END_DECLS

#endif
