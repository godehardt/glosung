/* autostart.c
 * Copyright (C) 2008-2010 Eicke Godehardt

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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "autostart.h"


static void init ();

static gchar *config_path = NULL;

#define AUTOSTART_BEGIN "\n[Desktop Entry]\nType=Application\nName=GLosung\n"
#define AUTOSTART_END "\nIcon=system-run\nComment=Watchwords for GNOME\nComment[de]=Losungen für GNOME\n"

GLosungAutostartType
is_in_autostart ()
{
	GLosungAutostartType result = GLOSUNG_NO_AUTOSTART;
        init ();

        if (g_file_test (config_path, G_FILE_TEST_IS_REGULAR)) {
        	result = GLOSUNG_AUTOSTART;

        	gchar *content;
                g_file_get_contents (config_path, &content, NULL, NULL);
        	if (strstr (content, "--once")) {
        		result = GLOSUNG_AUTOSTART_ONCE;
        	}
        	if (content) {
        		g_free (content);
        	}
        }
        return result;
} /* is_in_autostart */


/*
 * return TRUE on success
 */
gboolean
add_to_autostart (gboolean once)
{
        init ();

        if (once) {
                return g_file_set_contents (config_path,
			AUTOSTART_BEGIN "Exec=glosung --once"
			AUTOSTART_END, -1, NULL); /* TODO GError **error */
        } else {
                return g_file_set_contents (config_path,
			AUTOSTART_BEGIN "Exec=glosung"
			AUTOSTART_END, -1, NULL); /* TODO GError **error */
        }
} /* add_to_autostart */


/*
 * return TRUE on success
 */
gboolean
remove_from_autostart ()
{
        init ();

        if (remove (config_path) == -1) {
                g_message ("deletion of autostart file failed");
                return FALSE;
        }
        return TRUE;
} /* remove_from_autostart */


static void
init (void)
{
        /* TODO: const gchar* g_get_user_config_dir () and g_build_filename () */
        if (config_path) {
                return; /* config_path already set */
        }

        config_path = getenv ("XDG_CONFIG_HOME");
        if (config_path) {
                config_path = g_strdup_printf
                      ("%s/autostart/glosung.desktop", config_path);
        } else {
#ifndef WIN32
		config_path = g_strdup_printf
			  ("%s/.config/autostart/glosung.desktop", getenv ("HOME"));
#else /* WIN32 */
		config_path = g_strdup_printf ("%s%s/.config/autostart/glosung.desktop",
        		getenv ("HOMEDRIVE"), getenv ("HOMEPATH"));
#endif /* WIN32 */
        }
} /* init */
