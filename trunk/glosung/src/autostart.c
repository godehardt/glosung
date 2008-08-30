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
#include "autostart.h"


static void init ();

static gchar *config_path;


gboolean
is_in_autostart ()
{
        struct stat buf;

		init ();
		g_message ("%s", config_path);

        stat (config_path, &buf);
        return S_ISREG (buf.st_mode);
}


void
add_to_autostart ()
{
}


void
remove_from_autostart ()
{
}


static void
init (void)
{
        config_path = getenv ("XDG_CONFIG_HOME");
        if (config_path) {
                config_path = g_strdup_printf
						("%s/autostart/glosung.desktop",
						 config_path);
        } else {
                config_path = g_strdup_printf
						("%s/.config/autostart/glosung.desktop",
						 getenv ("HOME"));
        }
}
