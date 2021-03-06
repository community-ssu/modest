#include <libmodest-dbus-client/libmodest-dbus-client.h>
#include <stdio.h>
#include <glib.h>

int main (int argc, char *argv[])
{
	osso_context_t *osso_context;
	gboolean res;
	ModestDBusSearchFlags flags;
	GList *hits, *iter;
	const char *query;

	osso_context = osso_initialize ("test_search",
					"0.0.1",
					TRUE,
					NULL);


	/* Check that initialization was ok */
	if (osso_context == NULL) {
		g_printerr ("osso_initialize() failed.\n");
		return OSSO_ERROR;
	}

	hits = NULL;
	flags = MODEST_DBUS_SEARCH_SUBJECT | MODEST_DBUS_SEARCH_BODY;

	if (argc == 2) {
		query = argv[1];
	} else {
		query = "no";
	}

	g_print ("Starting search (%s)...\n", query);

	res = libmodest_dbus_client_search (osso_context,
					    query,
					    "",
					    0,
					    0,
					    0,
					    flags,
					    &hits);

	g_print ("Search done. (success: %s)\n", res ? "yes" : "no");

	for (iter = hits; iter; iter = iter->next) {
		ModestSearchHit *hit = (ModestSearchHit *) iter->data;

		g_print ("Hit: id: %s\n", hit->msgid);

	}

	modest_search_hit_list_free (hits);

	return res ? 0 : -1;
}
