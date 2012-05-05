/* settings.c
 * Copyright (C) 2010-2012 Eicke Godehardt

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

#include <gtk/gtk.h>

#include "settings.h"



#if GTK_CHECK_VERSION(2,26,0)
/* new gsettings */


GSettings *gsettings = NULL;


static void
init ()
{
	if (! gsettings) {
//		gsettings = g_settings_new ("org.godehardt.glosung");
		gsettings = g_settings_new_with_path ("org.godehardt.glosung", "//Users/d047370/gtk/inst/share/glib-2.0/schemas/gschemas.compiled");
	}
}


gboolean
is_proxy_in_use ()
{
	init ();
	return g_settings_get_boolean (gsettings, "/apps/" PACKAGE "/use_proxy");
} /* use_proxy */


void
set_proxy_in_use (gboolean use_proxy)
{
	init ();
	g_settings_set_boolean
		(gsettings, "/apps/" PACKAGE "/use_proxy", use_proxy);
} /* set_proxy_in_use */


gchar*
get_proxy ()
{
	init ();
        return g_settings_get_string
		(gsettings, "/apps/" PACKAGE "/proxy");
} /* get_proxy */


void
set_proxy (const gchar *proxy)
{
	init ();
	g_settings_set_string
		(gsettings, "/apps/" PACKAGE "/proxy", proxy);
} /* set_proxy */


gboolean
is_hide_warning ()
{
	init ();
	return g_settings_get_boolean (gsettings, "/apps/" PACKAGE "/hide_warning");
} /* is_hide_warning */


void
set_hide_warning (gboolean hide_warning)
{
	init ();
	g_settings_set_boolean
		(gsettings, "/apps/" PACKAGE "/hide_warning", hide_warning);
} /* set_hide_warning */


GDate*
get_last_usage ()
{
        GDate *last_time = NULL;
	init ();
        gchar *last_time_str = g_settings_get_string
                (gsettings, "/apps/" PACKAGE "/last_time");
        if (last_time_str) {
                last_time = g_date_new ();
                g_date_set_parse (last_time, last_time_str);
        }
        return last_time;
} /* get_last_usage */


void
set_last_usage (const GDate *date)
{
	init ();
        gchar *time_str = g_malloc (11); /* "YYYY-MM-DD"; */
        g_date_strftime (time_str, 11, "%Y-%m-%d", date);
        g_settings_set_string
                (gsettings, "/apps/" PACKAGE "/last_time", time_str);
} /* set_last_usage */


gchar*
get_language ()
{
	init ();
        return g_settings_get_string
		(gsettings, "/apps/" PACKAGE "/language");

} /* get_language */


void
set_language (const gchar *language)
{
	init ();
	g_settings_set_string
	     (gsettings, "/apps/" PACKAGE "/language", language);
} /* set_language */


gboolean
is_link_sword ()
{
	init ();
	return g_settings_get_boolean (gsettings, "/apps/" PACKAGE "/link_sword");
} /* is_link_sword */


void
set_link_sword (gboolean link_sword)
{
	init ();
	g_settings_set_boolean
		(gsettings, "/apps/" PACKAGE "/link_sword", link_sword);
} /* set_link_sword */


gchar*
get_font ()
{
	init ();
        return g_settings_get_string
		(gsettings, "/apps/" PACKAGE "/font");
} /* get_font */


void
set_font (const gchar *font)
{
	init ();
	g_settings_set_string
	     (gsettings, "/apps/" PACKAGE "/font", font);
} /* set_font */








/* ============================================================================= */

#else /* gconf vs. gsettings */
#if ! (defined (WIN32) || defined (__APPLE__))
/* old gconf */

#include <gconf/gconf-client.h>
#include <gtk/gtk.h>
#if (GTK_CHECK_VERSION(2,10,0))
	#define VERSE_LINK 1
#else
	#undef VERSE_LINK
#endif


static GConfClient *client = NULL;


static void
init ()
{
	if (! client) {
		client = gconf_client_get_default ();
	}
}


gboolean
is_proxy_in_use ()
{
	init ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/use_proxy", NULL);
} /* use_proxy */


void
set_proxy_in_use (gboolean use_proxy)
{
	init ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/use_proxy", use_proxy, NULL);
} /* set_proxy_in_use */


gchar*
get_proxy ()
{
	init ();
        return gconf_client_get_string
		(client, "/apps/" PACKAGE "/proxy", NULL);
} /* get_proxy */


void
set_proxy (const gchar *proxy)
{
	init ();
	gconf_client_set_string
		(client, "/apps/" PACKAGE "/proxy", proxy, NULL);
} /* set_proxy */


gboolean
is_hide_warning ()
{
	init ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/hide_warning", NULL);
} /* is_hide_warning */


void
set_hide_warning (gboolean hide_warning)
{
	init ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/hide_warning", hide_warning, NULL);
} /* set_hide_warning */


GDate*
get_last_usage ()
{
	init ();
        GDate *last_time = NULL;
        gchar *last_time_str = gconf_client_get_string
                (client, "/apps/" PACKAGE "/last_time", NULL);
        if (last_time_str) {
                last_time = g_date_new ();
                g_date_set_parse (last_time, last_time_str);
        }
        return last_time;
} /* get_last_usage */


void
set_last_usage (const GDate *date)
{
	init ();
        gchar *time_str = g_malloc (11); /* "YYYY-MM-DD"; */
        g_date_strftime (time_str, 11, "%Y-%m-%d", date);
        gconf_client_set_string
                (client, "/apps/" PACKAGE "/last_time", time_str, NULL);
} /* set_last_usage */


gchar*
get_language ()
{
	init ();
        return gconf_client_get_string
		(client, "/apps/" PACKAGE "/language", NULL);
} /* get_language */


void
set_language (const gchar *language)
{
	init ();
	gconf_client_set_string
	     (client, "/apps/" PACKAGE "/language", language, NULL);
} /* set_language */


gboolean
is_calender_double_click ()
{
	init ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/calendar_close_by_double_click", NULL);
} /* is_calender_double_click */


void
set_calender_double_click (gboolean calendar_close_by_double_click)
{
	init ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/calendar_close_by_double_click", calendar_close_by_double_click, NULL);
} /* set_calender_double_click */


gboolean
is_link_sword ()
{
	init ();
	return gconf_client_get_bool (client, "/apps/" PACKAGE "/link_sword", NULL);
} /* is_link_sword */


void
set_link_sword (gboolean link_sword)
{
	init ();
	gconf_client_set_bool
		(client, "/apps/" PACKAGE "/link_sword", link_sword, NULL);
} /* set_link_sword */


gchar*
get_font ()
{
	init ();
        return gconf_client_get_string
		(client, "/apps/" PACKAGE "/font", NULL);
} /* get_font */


void
set_font (const gchar *font)
{
	init ();
	gconf_client_set_string
	     (client, "/apps/" PACKAGE "/font", font, NULL);
} /* set_font */









/* ============================================================================= */
#else /* no configure at all (neither gconf nor gsettings) */


static gboolean  use_proxy = FALSE;
static gcher    *proxy = "";
static gboolean  hide_warning = FALSE;
static GDate    *last_use;
static gchar    *language = NULL;
static gboolean  link_sword = FALSE;
static gchar    *font = NULL;

gboolean
is_proxy_in_use ()
{
	return proxy;
} /* use_proxy */


void
set_proxy_in_use (gboolean _use_proxy)
{
	use_proxy = _use_proxy
} /* set_proxy_in_use */


gchar*
get_proxy ()
{
	return proxy;
} /* get_proxy */


void
set_proxy (const gchar *_proxy)
{
	proxy = _proxy;
} /* set_proxy */


gboolean
is_hide_warning ()
{
	return FALSE;
} /* is_hide_warning */


void
set_hide_warning (gboolean hide_warning)
{
} /* set_hide_warning */


GDate*
get_last_usage ()
{
        return last_use;
} /* get_last_usage */


void
set_last_usage (const GDate *date)
{
	last_use = date;
} /* set_last_usage */


gchar*
get_language ()
{
	return language;
} /* get_language */


void
set_language (const gchar *_language)
{
	language = _language
} /* set_language */


gboolean
is_link_sword ()
{
	return link_sword;
} /* is_link_sword */


void
set_link_sword (gboolean _link_sword)
{
	link_sword = _link_sword;
} /* set_link_sword */


gchar*
get_font ()
{
	return font;
} /* get_font */


void
set_font (const gchar *_font)
{
	font = _font;
} /* set_font */

#endif /* old gconf or nothing */
#endif /* gconf vs. gsettings */

