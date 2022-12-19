/* autostart.h
 * Copyright (C) 2008-2022 Eicke Godehardt

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

#ifndef GLOSUNG_AUTOSTART__H
#define GLOSUNG_AUTOSTART__H

#include <glib.h>

typedef enum {
	GLOSUNG_NO_AUTOSTART   = 0,
	GLOSUNG_AUTOSTART      = 1,
	GLOSUNG_AUTOSTART_ONCE = (GLOSUNG_AUTOSTART << 1) + 1
} GLosungAutostartType;

GLosungAutostartType  is_in_autostart       ();
gboolean              add_to_autostart      (gboolean once);
gboolean              remove_from_autostart ();


#endif /* GLOSUNG_AUTOSTART__H */
