/* settings.c
 * Copyright (C) 2010-2022 Eicke Godehardt

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
#ifdef WIN32
	#include "gconf-client.h"
#else
	#include <gconf/gconf-client.h>
#endif


#if (! defined (WIN32) && ! defined (__APPLE__))
  #define VERSE_LINK 1
#endif

static GConfClient *client = NULL;

#define INIT_CLIENT() \
	if (! client) { \
		client = gconf_client_get_default (); \
	}


gboolean
is_proxy_in_use ()
{
	INIT_CLIENT ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/use_proxy", NULL);
} /* use_proxy */


void
set_proxy_in_use (gboolean use_proxy)
{
	INIT_CLIENT ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/use_proxy", use_proxy, NULL);
} /* set_proxy_in_use */


gchar*
get_proxy ()
{
	INIT_CLIENT ();
	return gconf_client_get_string
		(client, "/apps/" PACKAGE "/proxy", NULL);
} /* get_proxy */


void
set_proxy (const gchar *proxy)
{
	INIT_CLIENT ();
	gconf_client_set_string
		(client, "/apps/" PACKAGE "/proxy", proxy, NULL);
} /* set_proxy */


gchar*
get_herrnhuter_url ()
{
        INIT_CLIENT ();
        return gconf_client_get_string
                (client, "/apps/" PACKAGE "/losungen-url", NULL);
} /* get_herrnhuter_url */


void
set_herrnhuter_url (const gchar *url)
{
        INIT_CLIENT ();
        gconf_client_set_string
                (client, "/apps/" PACKAGE "/losungen-url", url, NULL);
} /* set_herrnhuter_url */


gboolean
is_hide_warning ()
{
	INIT_CLIENT ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/hide_warning", NULL);
} /* is_hide_warning */


void
set_hide_warning (gboolean hide_warning)
{
	INIT_CLIENT ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/hide_warning", hide_warning, NULL);
} /* set_hide_warning */


GDateTime*
get_last_usage ()
{
    GDateTime *last_time = NULL;
	INIT_CLIENT ();
	gchar *last_time_str = gconf_client_get_string
			(client, "/apps/" PACKAGE "/last_time", NULL);
	if (last_time_str) {
		last_time = g_date_time_new_from_iso8601 (last_time_str, NULL);
	}
	g_free (last_time_str);
	return last_time;
} /* get_last_usage */


void
set_last_usage (const GDateTime *date)
{
	INIT_CLIENT ();
	gchar *time_str = g_date_time_format_iso8601 ((GDateTime *)date);
	gconf_client_set_string
			(client, "/apps/" PACKAGE "/last_time", time_str, NULL);
	g_free (time_str);
} /* set_last_usage */


gchar*
get_language ()
{
	INIT_CLIENT ();
	return gconf_client_get_string
		(client, "/apps/" PACKAGE "/language", NULL);
} /* get_language */


void
set_language (const gchar *language)
{
	INIT_CLIENT ();
	gconf_client_set_string
	     (client, "/apps/" PACKAGE "/language", language, NULL);
} /* set_language */


gboolean
is_calender_double_click ()
{
	INIT_CLIENT ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/calendar_close_by_double_click", NULL);
} /* is_calender_double_click */


void
set_calender_double_click (gboolean calendar_close_by_double_click)
{
	INIT_CLIENT ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/calendar_close_by_double_click", calendar_close_by_double_click, NULL);
} /* set_calender_double_click */


gboolean
is_link_sword ()
{
#ifdef VERSE_LINK
	INIT_CLIENT ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/link_sword", NULL);
#else
	return FALSE;
#endif /* VERSE_LINK */
} /* is_link_sword */


void
set_link_sword (gboolean link_sword)
{
#ifdef VERSE_LINK
	INIT_CLIENT ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/link_sword", link_sword, NULL);
#endif /* VERSE_LINK */
} /* set_link_sword */


gchar*
get_font ()
{
	INIT_CLIENT ();
	return gconf_client_get_string
		(client, "/apps/" PACKAGE "/font", NULL);
} /* get_font */


void
set_font (const gchar *font)
{
	INIT_CLIENT ();
	gconf_client_set_string
	     (client, "/apps/" PACKAGE "/font", font, NULL);
} /* set_font */