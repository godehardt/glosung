/* autostart.c
 * Copyright (C) 2008 Eicke Godehardt

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
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
}


void
add_to_autostart ()
{
        FILE *file = fopen (config_path, "w");

        init ();

        /* TODO: use g_file_set_contents instead */
        fprintf (file, "[Desktop Entry]\n");
        fprintf (file, "Version=1.0\n");
        fprintf (file, "Encoding=UTF-8\n");
        fprintf (file, "Name=GLosung\n");
        fprintf (file, "Comment=Watchwords for Gnome\n");
        fprintf (file, "Comment[de]=Losungen für GNOME\n");
        fprintf (file, "Exec=glosung --once\n");
        fprintf (file, "X-GNOME-Autostart-enabled=true\n");
}


void
remove_from_autostart ()
{
        init ();

        if (remove (config_path) == -1) {
                g_message ("deletion of autostart file failed");
        }
}


static void
init (void)
{
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
