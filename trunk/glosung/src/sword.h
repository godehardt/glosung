/* sword.h
 * Copyright (C) 2007 Eicke Godehardt

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

#ifndef GLOSUNG_SWORD__H
#define GLOSUNG_SWORD__H

#ifdef __cplusplus
extern "C" {
#endif

#include <glib/gtypes.h>

/* get_sword_text ("GerElb", 43, 3, 16); */
gchar * get_sword_text (gchar *name, int book, int chapter, int verse);

GPtrArray *get_bibles (void);


#ifdef __cplusplus
}
#endif

#endif /* GLOSUNG_SWORD__H */
