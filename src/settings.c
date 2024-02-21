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
#include <gio/gio.h>


#if (! defined (WIN32) && ! defined (__APPLE__))
  #define VERSE_LINK 1
#endif

static GSettings *settings = NULL;

#define INIT_SETTINGS() \
	if (! settings) { \
		settings = g_settings_new ("org.godehardt.glosung"); \
	}
	// settings = g_settings_new_with_path ("org.godehardt.GLosung", "/home/godehardt/DEVELOP/glosung/");


gboolean
is_proxy_in_use ()
{
	INIT_SETTINGS ();
	return g_settings_get_boolean (settings, "use-proxy");
} /* use_proxy */


void
set_proxy_in_use (gboolean use_proxy)
{
	INIT_SETTINGS ();
	g_settings_set_boolean (settings, "use-proxy", use_proxy);
} /* set_proxy_in_use */


gchar*
get_proxy ()
{
	INIT_SETTINGS ();
	return g_settings_get_string (settings, "proxy");
} /* get_proxy */


void
set_proxy (const gchar *proxy)
{
	INIT_SETTINGS ();
	g_settings_set_string (settings, "proxy", proxy);
} /* set_proxy */


gchar*
get_herrnhuter_url ()
{
        INIT_SETTINGS ();
        return g_settings_get_string (settings, "losungen-url");
} /* get_herrnhuter_url */


void
set_herrnhuter_url (const gchar *url)
{
        INIT_SETTINGS ();
        g_settings_set_string (settings, "losungen-url", url);
} /* set_herrnhuter_url */


gboolean
is_hide_warning ()
{
	INIT_SETTINGS ();
	return g_settings_get_boolean (settings, "hide-warning");
} /* is_hide_warning */


void
set_hide_warning (gboolean hide_warning)
{
	INIT_SETTINGS ();
	g_settings_set_boolean (settings, "hide-warning", hide_warning);
} /* set_hide_warning */


GDateTime*
get_last_usage ()
{
    GDateTime *last_time = NULL;
	INIT_SETTINGS ();
	gchar *last_time_str = g_settings_get_string
			(settings, "last-time-opened");
	if (last_time_str) {
		last_time = g_date_time_new_from_iso8601 (last_time_str, NULL);
	}
	g_free (last_time_str);
	return last_time;
} /* get_last_usage */


void
set_last_usage (const GDateTime *date)
{
	INIT_SETTINGS ();
	gchar *time_str = g_date_time_format_iso8601 ((GDateTime *)date);
	g_settings_set_string
			(settings, "last-time-opened", time_str);
	g_free (time_str);
} /* set_last_usage */


gchar*
get_language ()
{
	INIT_SETTINGS ();
	return g_settings_get_string (settings, "language");
} /* get_language */


void
set_language (const gchar *language)
{
	INIT_SETTINGS ();
	g_settings_set_string (settings, "language", language);
} /* set_language */


gboolean
is_calender_double_click ()
{
	INIT_SETTINGS ();
	return g_settings_get_boolean (settings, "calendar-close-by-double-click");
} /* is_calender_double_click */


void
set_calender_double_click (gboolean calendar_close_by_double_click)
{
	INIT_SETTINGS ();
	g_settings_set_boolean
		(settings, "calendar-close-by-double-click", calendar_close_by_double_click);
} /* set_calender_double_click */


gboolean
is_link_sword ()
{
#ifdef VERSE_LINK
	INIT_SETTINGS ();
	return g_settings_get_boolean (settings, "link-sword");
#else
	return FALSE;
#endif /* VERSE_LINK */
} /* is_link_sword */


void
set_link_sword (gboolean link_sword)
{
#ifdef VERSE_LINK
	INIT_SETTINGS ();
	g_settings_set_boolean
		(settings, "link-sword", link_sword);
#endif /* VERSE_LINK */
} /* set_link_sword */


gchar*
get_font ()
{
	INIT_SETTINGS ();
	return g_settings_get_string (settings, "font");
} /* get_font */


void
set_font (const gchar *font)
{
	INIT_SETTINGS ();
	g_settings_set_string (settings, "font", font);
} /* set_font */