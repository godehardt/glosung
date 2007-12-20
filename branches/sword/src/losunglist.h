/* losunglist.h
 * Copyright (C) 2006-2007 Eicke Godehardt

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

#ifndef GLOSUNG_LOSUNGLIST__H
#define GLOSUNG_LOSUNGLIST__H

#include <glib/ghash.h>
#include <glib/garray.h>


typedef struct _LosungList LosungList;

struct _LosungList
{
        /* hash table: "en" -> [2007, 2006, 1999 ...] */
        GHashTable *hash_table;
        /* list containing pointer to "en", "de" etc */
        GPtrArray  *languages;
        /* 2007 -> "de" */
        GHashTable *all_years;        
};

LosungList* losunglist_new       ();
void        losunglist_add       (LosungList *list, gchar *lang, gint year);
void        losunglist_finialize (LosungList *list);

#endif /* GLOSUNG_LOSUNGLIST__H */
