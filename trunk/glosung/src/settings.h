/* settings.h
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

#ifndef GLOSUNG_SETTINGS__H
#define GLOSUNG_SETTINGS__H

#include <glib.h>


gboolean get_use_proxy      ();
void     set_use_proxy      (gboolean use_proxy);
gchar*   get_proxy          ();
void     set_proxy          (const gchar *proxy);
gchar*   get_proxy_user     ();
void     set_proxy_user     (const gchar *proxy);
gchar*   get_proxy_password ();
void     set_proxy_password (const gchar *proxy);

#endif /* GLOSUNG_SETTINGS__H */
