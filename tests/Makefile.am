#
# Makefile.am
#

SUBDIRS = dbus_api

# TODO: Why is this line here?
MAINTAINERCLEANFILES    = Makefile.in

TESTS_ENVIRONMENT       = top_builddir=$(top_builddir)  \
			  CK_FORK=yes 			\
			  CK_VERBOSITY=verbose

TESTS                   =                           \
			check_text-utils            \
			check_modest-conf           \
			check_update-account        \
			check_modest-utils          \
			check_account-mgr           

noinst_PROGRAMS=				    \
			check_folder-xfer           \
			check_modest-conf	    \
			check_text-utils            \
			check_modest-utils          \
			check_update-account        \
			check_account-mgr

INCLUDES=\
	@CHECK_CFLAGS@ \
	$(MODEST_GSTUFF_CFLAGS)\
	$(MODEST_PLATFORM_CFLAGS)\
	$(MODEST_TOOLKIT_CFLAGS)\
	$(MODEST_PLUGIN_DEFINES)\
	$(MODEST_LIBTINYMAIL_GNOME_DESKTOP_CFLAGS) \
	$(MODEST_LIBTINYMAIL_MAEMO_CFLAGS) \
	-I${top_srcdir}/src/$(MODEST_TOOLKIT_DIR) \
	-I${top_srcdir}/src \
	-DPREFIX=\"@prefix@\" \
	-DMODESTLOCALEDIR=\""$(modestlocaledir)"\"
objects=\
	@CHECK_LIBS@ \
	$(MODEST_GSTUFF_LIBS) \
	$(MODEST_PLATFORM_LIBS) \
	$(MODEST_TOOLKIT_LIBS) \
        $(MODEST_LIBTINYMAIL_GNOME_DESKTOP_LIBS) \
        $(MODEST_LIBTINYMAIL_MAEMO_LIBS) \
        ${top_srcdir}/src/$(MODEST_TOOLKIT_DIR)/libmodest-ui.la \
        ${top_srcdir}/src/widgets/libmodest-widgets.la \
        ${top_srcdir}/src/dbus_api/libmodest-dbus-api.la \
	${top_srcdir}/src/libmodest.la


check_folder_xfer_SOURCES=\
	check_folder-transfer.c
check_folder_xfer_LDADD = $(objects)

check_update_account_SOURCES=\
	check_update-account.c
check_update_account_LDADD = $(objects)

check_modest_conf_SOURCES=\
	check_modest-conf.c
check_modest_conf_LDADD = $(objects)

check_modest_utils_SOURCES=\
	check_modest-utils.c
check_modest_utils_LDADD = $(objects)

check_text_utils_SOURCES=\
	check_text-utils.c
check_text_utils_LDADD = $(objects)

check_account_mgr_SOURCES=\
	check_account-mgr.c
check_account_mgr_LDADD = $(objects)
