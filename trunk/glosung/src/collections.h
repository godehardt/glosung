/* collections.h
 * Copyright (C) 2006-2009 Eicke Godehardt

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

#ifndef GLOSUNG_COLLECTION__H
#define GLOSUNG_COLLECTION__H

#include <glib.h>


typedef struct _Collection Collection;

struct _Collection
{
        /* e.g. "de" -> [2001, 2002, 2007] */
        GHashTable *hash_table;
        /* e.g. ["de, "en", "cz"] */
        GPtrArray  *languages;
};

Collection* collection_new       (void);
void        collection_add       (Collection *list, gchar *lang, gint year);
void        collection_finialize (Collection *list);
Collection* scan_for_collections (void);

#endif /* GLOSUNG_COLLECTION__H */
