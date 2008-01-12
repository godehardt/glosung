/* losunglist.c
 * Copyright (C) 2006-2008 Eicke Godehardt

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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <glib/gdir.h>
#include <glib/gmem.h>
#include <glib/gmessages.h>
#include <glib/gstrfuncs.h>

#include "losunglist.h"


static void sort_years (gpointer key, gpointer value, gpointer langs);
static gint int_comp (gconstpointer a, gconstpointer b);
static gint str_comp (gconstpointer a, gconstpointer b);

static void scan_for_languages_in_dir (gchar *dirname, LosungList *list);


LosungList*
losunglist_new ()
{
        LosungList *list = g_new (LosungList, 1);
        list->hash_table = g_hash_table_new (g_str_hash, g_str_equal);
        return list;
}


void
losunglist_add (LosungList *list, gchar *lang, gint year)
{
        GPtrArray *years = g_hash_table_lookup (list->hash_table, lang);
        if (years == NULL) {
                years = g_ptr_array_new ();
                g_hash_table_insert (list->hash_table, lang, years);
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
losunglist_finialize (LosungList *list)
{
        list->languages = g_ptr_array_new ();
        g_hash_table_foreach (list->hash_table, sort_years, list->languages);
        g_ptr_array_sort (list->languages, str_comp);
}


static void
sort_years (gpointer key, gpointer value, gpointer langs)
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
LosungList*
scan_for_files (void)
{
        gchar *dirname;
        LosungList *list = losunglist_new ();

        scan_for_languages_in_dir (GLOSUNG_DATA_DIR, list);
        dirname = g_strdup_printf ("%s%s", getenv ("HOME"), "/.glosung");
        scan_for_languages_in_dir (dirname, list);
        g_free (dirname);
        losunglist_finialize (list);

        printf ("Found languages: ");
        guint i = 0;
        for (i = 0; i < (list->languages)->len; i++) {
                printf ("%s ",
                        (gchar*) g_ptr_array_index (list->languages, i));
        }
        printf ("\n");

        return list;
} /* scan_for_files */


static gboolean
check_for_losung_file (gchar *name, int len, LosungList *list)
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
                losunglist_add (list, langu, year);
                return TRUE;
        }
        return FALSE;
}


static gboolean
check_for_theword_file (gchar *name, int len, LosungList *list)
{
        if ((strncmp (name + len -  4, ".twd", 4)) == 0) {
                gchar *langu = g_strndup (name, 2);
                int year = -1;
                if (sscanf (name + len - 8, "%d", &year) == 0) {
                        sscanf (name + 3, "%d", &year);
                }
                if (year != -1) {
                        losunglist_add (list, langu, year);
                        return TRUE;
                }
        }
        return FALSE;
}


static gboolean
check_for_original_losung_file (gchar *name, int len, LosungList *list)
{
        if ((strncmp (name, "Losungen Free", 13)) == 0
            && (strncmp (name + len -  4, ".xml", 4)) == 0)
        {
                gchar *langu = g_strdup ("de");
                int year = -1;
                sscanf (name + 14, "%d", &year);
                if (year != -1) {
                        losunglist_add (list, langu, year);
                        return TRUE;
                }
        }
        return FALSE;
}


static void
scan_for_languages_in_dir (gchar *dirname, LosungList *list) 
{
        if (access (dirname, F_OK | R_OK) != 0) {
                return;
        }
        GDir  *dir = g_dir_open (dirname, 0, NULL);
        const  gchar *name;

        while ((name = g_dir_read_name (dir)) != NULL) {
                int len = strlen (name);
                check_for_losung_file          ((gchar *)name, len, list);
                check_for_original_losung_file ((gchar *)name, len, list);
                check_for_theword_file         ((gchar *)name, len, list);
        }
} /* scan_for_languages_in_dir */
