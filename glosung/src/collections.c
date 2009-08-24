/* collections.c
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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>

#include "collections.h"


static void collect_and_sort_years
                     (gpointer key, gpointer value, gpointer langs);
static gint int_comp (gconstpointer a, gconstpointer b);
static gint str_comp (gconstpointer a, gconstpointer b);

static void scan_for_collections_in_dir (gchar *dirname, CollectionSource *list);


CollectionSource*
collection_new (CollectionSourceType type, gchar *name)
{
        CollectionSource *cs = g_new (CollectionSource, 1);
        cs->type = type;
        cs->name = name;
        cs->collections = g_hash_table_new (g_str_hash, g_str_equal);
        return cs;
}


void
collection_add (CollectionSource *list, gchar *lang, gint year)
{
        GPtrArray *years = g_hash_table_lookup (list->collections, lang);
        if (years == NULL) {
                years = g_ptr_array_new ();
                g_hash_table_insert (list->collections, lang, years);
        }

        int i;
        for (i = 0; i < years->len; i++) {
                if (year == GPOINTER_TO_INT (g_ptr_array_index (years, i))) {
                        return;
                }
        }

        g_ptr_array_add (years, GINT_TO_POINTER (year));
}


void
collection_finialize (CollectionSource *list)
{
        list->languages = g_ptr_array_new ();
        g_hash_table_foreach (list->collections, collect_and_sort_years,
                              list->languages);
        g_ptr_array_sort (list->languages, str_comp);
}


static void
collect_and_sort_years (gpointer key, gpointer value, gpointer langs)
{
        g_ptr_array_add (langs, key);
        g_ptr_array_sort (value, int_comp);
}


static gint
int_comp (gconstpointer a, gconstpointer b)
{
        return *((int*)b) - *((int*)a);
}


static gint
str_comp (gconstpointer a, gconstpointer b)
{
        return strcmp (*((gchar**)a), *((gchar**)b));
}




/*
 * This function will search for several kind of losung files in
 * global and local directory.
 */
CollectionSource*
scan_for_collections (void)
{
        gchar *dirname;
        CollectionSource *cs = collection_new (COLLECTION_SOURCE_LOCAL, NULL);

        scan_for_collections_in_dir (GLOSUNG_DATA_DIR, cs);
#ifndef WIN32
        dirname = g_strdup_printf ("%s%s", getenv ("HOME"), "/.glosung");
        scan_for_collections_in_dir (dirname, cs);
#else /* WIN32 */
        dirname = g_strdup_printf ("%s%s%s",
        		getenv ("HOMEDRIVE"), getenv ("HOMEPATH"), "/.glosung");
        scan_for_collections_in_dir (dirname, cs);
#endif /* WIN32 */
        g_free (dirname);
        collection_finialize (cs);

        printf ("Found languages: ");
        guint i = 0;
        for (i = 0; i < (cs->languages)->len; i++) {
                printf ("%s ",
                        (gchar*) g_ptr_array_index (cs->languages, i));
        }
        printf ("\n");

        return cs;
} /* scan_for_collections */


static gboolean
check_for_losung_file (const gchar *name, int len, CollectionSource *list)
{
        if ((len == 12 || len == 15)
            && (strncmp (name + len -  4, ".xml", 4)) == 0
            && (strncmp (name + len - 10, "_los", 4)) == 0
            && isdigit (name [len - 6])
            && isdigit (name [len - 5]))
        {
                gchar *langu = g_strndup (name, len - 10);
                int year = 1900
                        + (name [len - 6] - 48) * 10
                        + (name [len - 5] - 48);
                if (year < 1970) {
                        year += 100;
                }
                collection_add (list, langu, year);
                return TRUE;
        }
        return FALSE;
}


static gboolean
check_for_theword_file (const gchar *name, int len, CollectionSource *list)
{
        if ((strncmp (name + len -  4, ".twd", 4)) == 0) {
                gchar *langu = g_strndup (name, 2);
                int year = -1;
                if (sscanf (name + len - 8, "%d", &year) == 0) {
                        sscanf (name + 3, "%d", &year);
                }
                if (year != -1) {
                        collection_add (list, langu, year);
                        return TRUE;
                }
        }
        return FALSE;
}


static gboolean
check_for_original_losung_file (const gchar *name, int len, CollectionSource *list)
{
        if ((strncmp (name, "Losungen Free", 13)) == 0
            && (strncmp (name + len -  4, ".xml", 4)) == 0)
        {
                gchar *langu = g_strdup ("de");
                int year = -1;
                sscanf (name + 14, "%d", &year);
                if (year != -1) {
                        collection_add (list, langu, year);
                        return TRUE;
                }
        }
        return FALSE;
}


static void
scan_for_collections_in_dir (gchar *dirname, CollectionSource *list)
{
        if (access (dirname, F_OK | R_OK) != 0) {
                return;
        }
        GDir  *dir = g_dir_open (dirname, 0, NULL);
        const gchar *name;

        while ((name = g_dir_read_name (dir)) != NULL) {
                int len = strlen (name);
                check_for_losung_file          (name, len, list);
                check_for_original_losung_file (name, len, list);
                check_for_theword_file         (name, len, list);
        }
} /* scan_for_languages_in_dir */


GPtrArray*
collectionsource_get_years (CollectionSource* cs, gchar *language)
{
        return g_hash_table_lookup (cs->collections, language);
}
