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

#ifndef GLOSUNG_COLLECTIONS__H
#define GLOSUNG_COLLECTIONS__H

#include <glib.h>


typedef enum {
        COLLECTION_SOURCE_LOCAL,
        COLLECTION_SOURCE_LOSUNGEN,
        COLLECTION_SOURCE_BIBLE20
} CollectionSourceType;

typedef struct {
        CollectionSourceType type;
        gchar *name;

        /* Mapping to sorted list of collections,
         * e.g. "de" -> [2001, 2002, 2007]. */
        GHashTable *collections;
        /* Sorted list of languages, e.g. ["de, "en", "cz"].
         * May be NULL, before finalize is called! */
        GPtrArray  *languages;
} CollectionSource;

typedef struct {
        CollectionSourceType type;
        gchar *language;
        gint   year;
        gchar *bible;
        gchar *biblename;
        GDate  updated;
        gchar *info;
        gchar *url;
} Collection;


CollectionSource* collection_new (CollectionSourceType type, gchar *name);
void        collection_add       (CollectionSource *cs, gchar *lang, gint year);
void        collection_finialize (CollectionSource *cs);
CollectionSource* scan_for_collections (void);

GPtrArray* collectionsource_get_years (CollectionSource* cs, gchar *language);

#endif /* GLOSUNG_COLLECTIONS__H */
