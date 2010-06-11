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

#include "autostart.h"


static void init ();

static gchar *config_path = NULL;


gboolean
is_in_autostart ()
{
        init ();

        return g_file_test (config_path, G_FILE_TEST_IS_REGULAR);
        /* TODO g_file_get_contents (config_path, gchar **contents,
                                gsize *length, NULL); */
}


/*
 * return TRUE on success
 */
gboolean
add_to_autostart (gboolean once)
{
        init ();

        return g_file_set_contents (config_path,
                 "\n[Desktop Entry]\nType=Application\nName=GLosung\n"
                 "Exec=glosung --once\nIcon=system-run\n"
                 "Comment=Watchwords for GNOME\nComment[de]=Losungen für GNOME",
                 -1, NULL); /* TODO GError **error */
}


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
}


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
}
