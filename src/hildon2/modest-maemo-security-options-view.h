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

#ifndef __MODEST_MAEMO_SECURITY_OPTIONS_VIEW_H__
#define __MODEST_MAEMO_SECURITY_OPTIONS_VIEW_H__

#include <glib-object.h>
#include "modest-security-options-view.h"
#include <gtk/gtkbox.h>
#include <gtk/gtksizegroup.h>

G_BEGIN_DECLS

/* convenience macros */
#define MODEST_TYPE_MAEMO_SECURITY_OPTIONS_VIEW             (modest_maemo_security_options_view_get_type())
#define MODEST_MAEMO_SECURITY_OPTIONS_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MODEST_TYPE_MAEMO_SECURITY_OPTIONS_VIEW,ModestMaemoSecurityOptionsView))
#define MODEST_MAEMO_SECURITY_OPTIONS_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MODEST_TYPE_MAEMO_SECURITY_OPTIONS_VIEW,GObject))
#define MODEST_IS_MAEMO_SECURITY_OPTIONS_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MODEST_TYPE_MAEMO_SECURITY_OPTIONS_VIEW))
#define MODEST_IS_MAEMO_SECURITY_OPTIONS_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MODEST_TYPE_MAEMO_SECURITY_OPTIONS_VIEW))
#define MODEST_MAEMO_SECURITY_OPTIONS_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MODEST_TYPE_MAEMO_SECURITY_OPTIONS_VIEW,ModestMaemoSecurityOptionsViewClass))

typedef struct _ModestMaemoSecurityOptionsView      ModestMaemoSecurityOptionsView;
typedef struct _ModestMaemoSecurityOptionsViewClass ModestMaemoSecurityOptionsViewClass;

struct _ModestMaemoSecurityOptionsView {
	 ModestSecurityOptionsView parent;
};

struct _ModestMaemoSecurityOptionsViewClass {
	ModestSecurityOptionsViewClass parent_class;
};

/* member functions */
GType        modest_maemo_security_options_view_get_type    (void) G_GNUC_CONST;

GtkWidget* modest_maemo_security_options_view_new (ModestSecurityOptionsType type,
						   gboolean full,
						   GtkSizeGroup *title_size_group,
						   GtkSizeGroup *value_size_group);

G_END_DECLS

#endif /* __MODEST_MAEMO_SECURITY_OPTIONS_VIEW_H__ */

