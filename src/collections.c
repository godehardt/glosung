/* collections.c
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


#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "collections.h"
#include "download.h"


static GPtrArray *sources = NULL;


static void collect_and_sort_years
                     (gpointer key, gpointer value, gpointer langs);
static gint versecollection_comp (gconstpointer a, gconstpointer b);
static gint str_comp             (gconstpointer a, gconstpointer b);

static void scan_for_collections        (Source* cs);
static void scan_for_collections_in_dir (gchar *dirname, Source *list);


static int year = 0;



Source*
source_new (SourceType type, gchar *name)
{
        Source *cs = g_new (Source, 1);
        cs->type = type;
        cs->name = name;
        cs->collections = g_hash_table_new (g_str_hash, g_str_equal);
        cs->languages = NULL;
        return cs;
}


void
source_finialize (Source *list)
{
        list->languages = g_ptr_array_new ();
        g_hash_table_foreach (list->collections, collect_and_sort_years,
                              list->languages);
        g_ptr_array_sort (list->languages, str_comp);
}


VerseCollection*
source_add_collection (Source *cs, gchar *lang, gint year)
{
        VerseCollection *result;

        GPtrArray *vc_s = g_hash_table_lookup (cs->collections, lang);
        if (vc_s == NULL) {
                vc_s = g_ptr_array_new ();
                g_hash_table_insert (cs->collections, lang, vc_s);
        }

        for (gint i = 0; i < vc_s->len; i++) {
        	result = VC (g_ptr_array_index (vc_s, i));
                if (year == result->year) {
                        return result;
                }
        }

        result = g_new (VerseCollection, 1);
        result->language = lang;
        result->year     = year;

        g_ptr_array_add (vc_s, result);
        return result;
} /* source_add_collection */


static void
collect_and_sort_years (gpointer key, gpointer value, gpointer langs)
{
        g_ptr_array_add (langs, key);
        g_ptr_array_sort (value, versecollection_comp);
}


static gint
versecollection_comp (gconstpointer a, gconstpointer b)
{
        return VC(*((VerseCollection**)b))->year
	     - VC(*((VerseCollection**)a))->year;
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
Source*
get_local_collections (void)
{
        get_sources ();
        Source *cs = (Source*)
                g_ptr_array_index (sources, SOURCE_LOCAL);
        scan_for_collections (cs);

        return cs;
} /* get_local_collections */


static void
scan_for_collections (Source* cs)
{
        gchar *dirname;

        scan_for_collections_in_dir (GLOSUNG_DATA_DIR, cs);
        dirname = g_build_filename (g_get_home_dir (), ".glosung", NULL);
        scan_for_collections_in_dir (dirname, cs);
        g_free (dirname);
        source_finialize (cs);

        printf ("Found languages: ");
        for (gint i = 0; i < (cs->languages)->len; i++) {
                printf ("%s ",
                        (gchar*) g_ptr_array_index (cs->languages, i));
        }
        printf ("\n");
} /* scan_for_collections */


static gboolean
check_for_losung_file (const gchar *name, int len, Source *cs)
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
                VerseCollection *vc = source_add_collection (cs, langu, year);
                struct stat buffer;
                int status = g_stat (name, &buffer);
                if (status == 0) {
                	GDate *updated = g_date_new ();
                	g_date_set_time_t (updated, buffer.st_mtime);
                	vc->updated = updated;
                }
                return TRUE;
        }
        return FALSE;
}


static gboolean
check_for_theword_file (const gchar *name, int len, Source *cs)
{
        if ((strncmp (name + len -  4, ".twd", 4)) == 0) {
                gchar *langu = g_strndup (name, 2);
                int year = -1;
                if (sscanf (name + len - 8, "%d", &year) == 0) {
                        sscanf (name + 3, "%d", &year);
                }
                if (year != -1) {
                        VerseCollection *vc = source_add_collection (cs, langu, year);
                        struct stat buffer;
                        int status = g_stat (name, &buffer);
                        if (status == 0) {
                        	GDate *updated = g_date_new ();
                        	g_date_set_time_t (updated, buffer.st_mtime);
                        	vc->updated = updated;
                        }
                        return TRUE;
                }
        }
        return FALSE;
}


static gboolean
check_for_original_losung_file (const gchar *name, int len, Source *cs)
{
        if ((g_ascii_strncasecmp (name, "Losungen Free", 13)) == 0
            && (g_ascii_strncasecmp (name + len -  4, ".xml", 4)) == 0)
        {
                gchar *langu = g_strdup ("de");
                int year = -1;
                sscanf (name + 14, "%d", &year);
                if (year != -1) {
                        VerseCollection *vc = source_add_collection (cs, langu, year);
                        struct stat buffer;
                        int status = g_stat (name, &buffer);
                        if (status == 0) {
                        	GDate *updated = g_date_new ();
                        	g_date_set_time_t (updated, buffer.st_mtime);
                        	vc->updated = updated;
                        }
                        return TRUE;
                }
        }
        return FALSE;
} /* check_for_original_losung_file */


static void
scan_for_collections_in_dir (gchar *dirname, Source *cs)
{
        if (access (dirname, F_OK | R_OK) != 0) {
                return;
        }
        GDir  *dir = g_dir_open (dirname, 0, NULL);
        const gchar *name;

        while ((name = g_dir_read_name (dir)) != NULL) {
                int len = strlen (name);
                check_for_losung_file          (name, len, cs);
                check_for_original_losung_file (name, len, cs);
                check_for_theword_file         (name, len, cs);
        }
} /* scan_for_languages_in_dir */


GPtrArray*
get_sources ()
{
        if (sources == NULL) {
                sources = g_ptr_array_sized_new (3);
                Source *cs;

                cs = source_new (SOURCE_LOCAL, NULL);
                g_ptr_array_add (sources, cs);

                cs = source_new (SOURCE_LOSUNGEN, _("Herrnhuter Losungen"));
                g_ptr_array_add (sources, cs);

                cs = source_new (SOURCE_BIBLE20,  _("Bible 2.0"));
                g_ptr_array_add (sources, cs);
        }
        return sources;
}


GPtrArray*
source_get_languages (Source* cs)
{
        if (cs == NULL) {
                g_message ("Source is NULL!");
                return NULL;
        }
        if (cs->languages == NULL) {
                switch (cs->type) {
                case SOURCE_LOCAL:
                        scan_for_collections (cs);
                        break;
                case SOURCE_LOSUNGEN:
                	if (! year) {
                                time_t     t;
                                struct tm  now;

                                t = time (NULL);
                                now = *localtime (&t);
                                year = now.tm_year + 1900;
                        	// from October offer download next year's texts
                                if (now.tm_mon >= 9) {
                                	year++;
                                }
                	}

                	for (gint i = 0; i < 3; i++) {
        			source_add_collection (cs, "de", year - i);
                	}
			source_finialize (cs);
                	break;
                case SOURCE_BIBLE20:
                	get_bible20_collections (cs);
                        break;
                }
        }
        return cs->languages;
}


GPtrArray*
source_get_collections (const Source* cs, const gchar *language)
{
	if (cs == NULL) {
                g_message ("Source is NULL!");
                return NULL;
        }
        return g_hash_table_lookup (cs->collections, language);
}
