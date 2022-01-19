/* settings.h
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

#ifndef GLOSUNG_SETTINGS__H
#define GLOSUNG_SETTINGS__H

#include <glib.h>


gboolean   is_proxy_in_use    ();
void       set_proxy_in_use   (gboolean use_proxy);
gchar*     get_proxy          ();
void       set_proxy          (const gchar *proxy);
gchar*     get_herrnhuter_url ();
void       set_herrnhuter_url (const gchar *url);
gboolean   is_hide_warning    ();
void       set_hide_warning   (gboolean hide_warning);
GDateTime* get_last_usage     ();
void       set_last_usage     (const GDateTime *date);
gchar*     get_language       ();
void       set_language       (const gchar *language);
gboolean   is_calender_double_click ();
void       set_calender_double_click (gboolean calendar_close_by_double_click);
gboolean   is_link_sword      ();
void       set_link_sword     (gboolean link_sword);
gchar*     get_font           ();
void       set_font           (const gchar *font);

#endif /* GLOSUNG_SETTINGS__H */
