/* settings.c
 * Copyright (C) 2010-2016 Eicke Godehardt

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
	#include <gtk/gtk.h>
#endif


#if (GTK_MINOR_VERSION >= (10) && ! defined (WIN32))
  #define VERSE_LINK 1
#endif

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
#else
	return FALSE;
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
#else
	return "";
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


gboolean
is_hide_warning ()
{
#ifndef WIN32
	INIT_CLIENT ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/hide_warning", NULL);
#else
	return FALSE;
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


GDate*
get_last_usage ()
{
        GDate *last_time = NULL;
#ifndef WIN32
	INIT_CLIENT ();
        gchar *last_time_str = gconf_client_get_string
                (client, "/apps/" PACKAGE "/last_time", NULL);
        if (last_time_str) {
                last_time = g_date_new ();
                g_date_set_parse (last_time, last_time_str);
        }
#endif /* WIN32 */
        return last_time;
} /* get_last_usage */


void
set_last_usage (const GDate *date)
{
#ifndef WIN32
	INIT_CLIENT ();
        gchar *time_str = g_malloc (11); /* "YYYY-MM-DD"; */
        g_date_strftime (time_str, 11, "%Y-%m-%d", date);
        gconf_client_set_string
                (client, "/apps/" PACKAGE "/last_time", time_str, NULL);
#endif /* WIN32 */
} /* set_last_usage */


gchar*
get_language ()
{
#ifndef WIN32
	INIT_CLIENT ();
        return gconf_client_get_string
		(client, "/apps/" PACKAGE "/language", NULL);
#else
	return NULL;
#endif /* WIN32 */
} /* get_language */


void
set_language (const gchar *language)
{
#ifndef WIN32
	INIT_CLIENT ();
	gconf_client_set_string
	     (client, "/apps/" PACKAGE "/language", language, NULL);
#endif /* WIN32 */
} /* set_language */


gboolean
is_calender_double_click ()
{
#ifndef WIN32
	INIT_CLIENT ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/calendar_close_by_double_click", NULL);
#else
	return FALSE;
#endif /* WIN32 */
} /* is_calender_double_click */


void
set_calender_double_click (gboolean calendar_close_by_double_click)
{
#ifndef WIN32
	INIT_CLIENT ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/calendar_close_by_double_click", calendar_close_by_double_click, NULL);
#endif /* WIN32 */
} /* set_calender_double_click */


gboolean
is_link_sword ()
{
#if (defined (VERSE_LINK) && ! defined (WIN32))
	INIT_CLIENT ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/link_sword", NULL);
#else
	return FALSE;
#endif /* VERSE_LINK, WIN32 */
} /* is_link_sword */


void
set_link_sword (gboolean link_sword)
{
#if (defined (VERSE_LINK) && ! defined (WIN32))
	INIT_CLIENT ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/link_sword", link_sword, NULL);
#endif /* VERSE_LINK, WIN32 */
} /* set_link_sword */


gchar*
get_font ()
{
#ifndef WIN32
	INIT_CLIENT ();
        return gconf_client_get_string
		(client, "/apps/" PACKAGE "/font", NULL);
#else
	return NULL;
#endif /* WIN32 */
} /* get_font */


void
set_font (const gchar *font)
{
#ifndef WIN32
	INIT_CLIENT ();
	gconf_client_set_string
	     (client, "/apps/" PACKAGE "/font", font, NULL);
#endif /* WIN32 */
} /* set_font */
