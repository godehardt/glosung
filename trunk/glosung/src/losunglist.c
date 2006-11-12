/* losunglist.c
 * Copyright (C) 2006 Eicke Godehardt

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

#include <string.h>
#include <glib/gmem.h>

#include "losunglist.h"


static void sort_years (gpointer key, gpointer value, gpointer langs);
static gint int_comp (gconstpointer a, gconstpointer b);
static gint str_comp (gconstpointer a, gconstpointer b);


LosungList*
losunglist_init ()
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
