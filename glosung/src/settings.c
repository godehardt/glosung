/* settings.c
 * Copyright (C) 2010 Eicke Godehardt

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/* FIXME */
#define PACKAGE "glosung"

#include "settings.h"
#ifndef WIN32
	#include <gconf/gconf-client.h>
#endif

/*
static gboolean   use_proxy = FALSE;
static gchar     *proxy = NULL;
*/
#ifndef WIN32
	static GConfClient *client = NULL;
#endif /* WIN32 */

#define INIT_CLIENT() \
	if (! client) { \
		client = gconf_client_get_default (); \
	}


gboolean
is_proxy_in_use ()
{
#ifndef WIN32
	INIT_CLIENT ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/use_proxy", NULL);
#endif /* WIN32 */
} /* use_proxy */


void
set_proxy_in_use (gboolean use_proxy)
{
#ifndef WIN32
	INIT_CLIENT ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/use_proxy", use_proxy, NULL);
#endif /* WIN32 */
} /* set_proxy_in_use */


gchar*
get_proxy ()
{
#ifndef WIN32
	INIT_CLIENT ();
        return gconf_client_get_string
        		(client, "/apps/" PACKAGE "/proxy", NULL);
#endif /* WIN32 */
} /* get_proxy */


void
set_proxy (const gchar *proxy)
{
#ifndef WIN32
	INIT_CLIENT ();
	gconf_client_set_string
		(client, "/apps/" PACKAGE "/proxy", proxy, NULL);
#endif /* WIN32 */
} /* set_proxy */


gchar*
get_proxy_user ()
{
#ifndef WIN32
	INIT_CLIENT ();
        return gconf_client_get_string
        		(client, "/apps/" PACKAGE "/proxy_user", NULL);
#endif /* WIN32 */
} /* get_proxy_user */


void
set_proxy_user (const gchar *proxy_user)
{
#ifndef WIN32
	INIT_CLIENT ();
	gconf_client_set_string
		(client, "/apps/" PACKAGE "/proxy_user", proxy_user, NULL);
#endif /* WIN32 */
} /* set_proxy_user */


gchar*
get_proxy_password ()
{
#ifndef WIN32
	INIT_CLIENT ();
        return gconf_client_get_string
        		(client, "/apps/" PACKAGE "/proxy_password", NULL);
#endif /* WIN32 */
} /* get_proxy_password */


void
set_proxy_password (const gchar *proxy_password)
{
#ifndef WIN32
	INIT_CLIENT ();
	gconf_client_set_string
	     (client, "/apps/" PACKAGE "/proxy_password", proxy_password, NULL);
#endif /* WIN32 */
} /* set_proxy_password */


gboolean
is_hide_warning ()
{
#ifndef WIN32
	INIT_CLIENT ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/hide_warning", NULL);
#endif /* WIN32 */
} /* is_hide_warning */


void
set_hide_warning (gboolean hide_warning)
{
#ifndef WIN32
	INIT_CLIENT ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/hide_warning", hide_warning, NULL);
#endif /* WIN32 */
} /* set_hide_warning */
