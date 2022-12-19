/* collections.h
 * Copyright (C) 2006-2022 Eicke Godehardt

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

G_BEGIN_DECLS

typedef enum {
        SOURCE_LOCAL,
        SOURCE_LOSUNGEN,
        SOURCE_BIBLE20
} SourceType;

typedef struct {
        SourceType type;
        gchar *name;

        /* Mapping to ordered list of collections,
         * e.g. "de" -> [2001, 2002, 2007]. */
        GHashTable *collections;
        /* Ordered list of languages, e.g. ["de, "en", "cz"].
         * May be NULL, before finalize is called! */
        GPtrArray  *languages;
} Source;

typedef struct {
        SourceType type;
        gchar *language;
        gint   year;
        gchar *bible;
        GDate *updated;
        gchar *info;
        gchar *url;
} VerseCollection;

#define SOURCE(obj)		  ((Source*) obj)
#define CS			  SOURCE
#define VERSION_COLLECTION(obj)	  ((VerseCollection*) obj)
#define VC			  VERSION_COLLECTION

Source* source_new                (SourceType type, gchar *name);
void    source_finialize          (Source *cs);

GPtrArray* get_sources            (void);
GPtrArray* source_get_languages   (Source* cs);
GPtrArray* source_get_collections (const Source* cs, const gchar *language);

Source*    get_local_collections  (void);


VerseCollection* source_add_collection (Source *cs, gchar *language, gint year);
/*
VerseCollection* collection_add_full  (Source *cs,
                                       gchar *language, gint   year,
                                       gchar *bible,    GDate *updated,
                                       gchar *info,     gchar *url);
*/

G_END_DECLS

#endif /* GLOSUNG_COLLECTIONS__H */
