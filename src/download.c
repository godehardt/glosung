/* download.c
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
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>
#include <errno.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

//#include <curl/types.h>
//#include <curl/easy.h>

#include "download.h"
#include "collections.h"
#include "settings.h"


//#define LOSUNGEN_URL "http://www.brueder-unitaet.de/download/Losung_%d_XML.zip"
#define LOSUNGEN_URL "https://www.losungen.de/fileadmin/media-losungen/download/Losung_%d_XML.zip"
#define BIBLE20_BASE_URL "http://bible2.net/service/TheWord/twd11/?format=csv"

static gchar      *glosung_dir = NULL;
static int         error = 0;
static GHashTable *error_messages = NULL;

typedef struct Memory {
        char *memory;
        size_t size;
} Memory;


static void   init     ();
static void   to_file_losungen     (Memory mem, const guint year);
static Memory real_download        (const gchar *url);
static void   analyse_bible20_list (Source* cs, Memory mem);
static void   download_losungen    (guint year);
static void   download_bible20     (const Source* cs, const gchar *lang, guint year);
static void   init_error_messages  ();


static size_t
WriteMemoryCallback (void *ptr, size_t size, size_t nmemb, void *data)
{
        size_t realsize = size * nmemb;
        Memory *mem = (Memory *)data;

        mem->memory = (char *)g_realloc (mem->memory,
                                         mem->size + realsize + 1);
        if (mem->memory) {
                memcpy (& (mem->memory [mem->size]), ptr, realsize);
                mem->size += realsize;
                mem->memory [mem->size] = 0;
        }
        return realsize;
}


static void
to_file (Memory chunk, gchar *filename)
{
        FILE *file = fopen (filename, "wb");

        if (file) {
                size_t written = fwrite (chunk.memory, 1, chunk.size, file);
                fclose (file);
                if (written != chunk.size) {
                        g_message ("download error - bytes written: %d",
                                   (int) written);
                        remove (filename);
                        error = -2;
                }
        }
}


static void
to_file_losungen (Memory mem, const guint year)
{
        init ();

        gchar *tmp_dir = g_strdup_printf ("%s/tmp_download", glosung_dir);
        gchar *zipfile = g_strdup_printf ("%s/Losung_XML.zip", tmp_dir);

        g_mkdir (tmp_dir, 0777);
        to_file (mem, zipfile);
        if (error != 0) {
                rmdir (tmp_dir);
                g_free (tmp_dir);
                g_free (zipfile);
                return;
        }

        // TODO: use getcwd (char *buf, size_t size); and reset afterwards
        if (chdir (tmp_dir) != 0) {
                error = -4;
        }
        gchar *command = g_strdup_printf ("unzip %s", zipfile);
        // TODO: handle exit status and error code
        gboolean success =
                g_spawn_command_line_sync (command, NULL, NULL, NULL, NULL);
        g_free (command);
        g_free (zipfile);
        if (! success) {
                g_message ("error while unzip");
                error = -3;
        }

        GDir *dir = g_dir_open (tmp_dir, 0, NULL);
        const gchar *name;
        while ((name = g_dir_read_name (dir))) {
                if ((strncmp (name + strlen (name) -  4, ".xml", 4)) == 0) {
                        gchar *losung_file =
                                g_strdup_printf ("%s/%s", tmp_dir, name);
                        gchar *target_file =
                                g_strdup_printf ("%s/Losungen Free %d.xml",
                                		glosung_dir, year);
                        // g_strdup_printf ("%s/%s", glosung_dir, name);
                        if (rename (losung_file, target_file)) {
                                perror (NULL);
                        }
                        g_free (losung_file);
                        g_free (target_file);
                } else {
                        remove (name);
                }
        }

        rmdir (tmp_dir);
        g_free (tmp_dir);
}


static void
download_losungen (guint year)
{
        gchar *url;
        url = get_herrnhuter_url ();
        if (! url) {
                url = LOSUNGEN_URL;
        }
        url = g_strdup_printf (url, year);
        Memory mem = real_download (url);
	g_free (url);

        if (error == 0) {
		to_file_losungen (mem, year);
        }
        if (mem.memory) {
        	g_free (mem.memory);
        }
}


static void
download_bible20  (const Source *cs, const gchar *lang, guint year)
{
        GPtrArray* css = source_get_collections (cs, lang);
	VerseCollection *vc = NULL;
        for (gint i = 0; i < css->len; i++) {
        	vc = (VerseCollection*) g_ptr_array_index (css, i);
        	if (vc->year == year) {
        		break;
        	} else {
        		vc = NULL;
        	}
        }

        Memory mem = real_download (vc->url);
        if (error == 0) {
		init ();
		gchar *filename =
			g_strdup_printf ("%s%s", glosung_dir, strrchr (vc->url, '/'));

		to_file (mem, filename);
		g_free (filename);
        }
        if (mem.memory) {
        	g_free (mem.memory);
        }
}


/*
 * generic method to download specified url to chunk of memory
 */
static Memory
real_download (const gchar *url)
{
        CURL     *curl_handle;
        /* CURLcode  res = -1; */
        Memory    chunk;

        chunk.memory = NULL;
        chunk.size   = 0;

        curl_global_init (CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init ();
        if (curl_handle) {
                curl_easy_setopt (curl_handle, CURLOPT_URL, url);
                curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION,
                                  WriteMemoryCallback);
                curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA,
                                  (void *)&chunk);
                curl_easy_setopt (curl_handle, CURLOPT_USERAGENT,
                                  "glosung/" VERSION);
		gchar *proxy = get_proxy();
		if (is_proxy_in_use () && proxy && strlen (proxy) > 0) {
			curl_easy_setopt (curl_handle, CURLOPT_PROXY, proxy);
			/*
			gchar *proxy_user = get_proxy_user ();
			if (proxy_user && strlen (proxy_user) > 0) {
				gchar *str;
				gchar *proxy_password = get_proxy_password ();
				if (proxy_password) {
				        str = g_strdup_printf
				                ("%s:%s", proxy_user, proxy_password);
				} else {
				        str = g_strdup_printf
				                ("%s:", proxy_user);
				}
				curl_easy_setopt (curl_handle,
						CURLOPT_PROXYUSERPWD, str);
				g_free (str);
			}
			*/
		}
		error = curl_easy_perform (curl_handle);
		curl_easy_cleanup (curl_handle);
	} else {
		error = -1;
	}
        return chunk;
} /* real download */


static void
analyse_bible20_list (Source* cs, Memory mem)
{
	g_assert (cs->type == SOURCE_BIBLE20);
	g_assert (mem.size > 0);

	gchar** lines = g_strsplit (mem.memory, "\n", -1);
        gint col_year    = 0;
        gint col_lang    = 0;
        gint col_url     = 0;
        gint col_bible   = 0;
        gint col_info    = 0;
        /* gint col_updated = 0; */

        gint j = 0;
        gint i = 0;

        gchar** tokens = g_strsplit (lines [j], ";", -1);
        while (tokens [i]) {
                gchar* token = tokens [i];
                if        (g_str_equal ("year",    token)) {
                        col_year = i;
                } else if (g_str_equal ("lang",    token)) {
                        col_lang = i;
                } else if (g_str_equal ("url",     token)) {
                        col_url = i;
                } else if (g_str_equal ("bible",   token)) {
                        col_bible = i;
                } else if (g_str_equal ("info",    token)) {
                        col_info = i;
                } /* else if (g_str_equal ("updated", token)) {
                        col_updated = i;
                } */
                i++;
        }
        g_strfreev (tokens);
        if (! col_year || ! col_lang || ! col_url) {
                g_message ("Syntax error in first line of CSV!");
                error = -42;
                return;
        }

        j++;
        while (lines [j]) {
                tokens = g_strsplit (lines [j], ";", -1);
                if (tokens [0]) {
			if (g_str_equal ("file", tokens [0])) {
				i = 0;
				guint year;
				sscanf (tokens [col_year], "%u", &year);
				VerseCollection* vc = source_add_collection
					(cs, tokens [col_lang], year);
				vc->url     = tokens [col_url];
				vc->bible   = tokens [col_bible];
				vc->info    = tokens [col_info];
				// vc->updated = g_date_new ();
				// g_date_set_parse (vc->updated, tokens [col_updated]);
			}
			g_free (tokens [0]);
			g_free (tokens [col_year]);
			g_free (tokens);
                }
                j++;
        }
        g_strfreev (lines);

        source_finialize (cs);
} /* analyse_bible20_list */


int
get_bible20_collections (Source* cs)
{
        error = 0;
        Memory mem = real_download (BIBLE20_BASE_URL);
        if (mem.memory) {
		analyse_bible20_list (cs, mem);
		g_free (mem.memory);
        }

        // FIXME return real values (check if "error" is set appropriately)
        return error;
}


int
download (const Source *cs, const gchar *language, guint year)
{
        error = 0;
	switch (cs->type) {
	case SOURCE_LOSUNGEN:
		download_losungen (year);
		break;
	case SOURCE_BIBLE20:
		download_bible20 (cs, language, year);
		break;
	default:
		g_message ("unknown source type '%s'", cs->name);
		error = -100;
	}

        return error;
} /* download */


static void
init (void)
{
        if (glosung_dir) {
                return; /* config_path already set */
        }

        glosung_dir = g_strdup_printf
                ("%s/.glosung", getenv ("HOME"));
        if (! g_file_test (glosung_dir, G_FILE_TEST_IS_DIR)) {
                int error = g_mkdir (glosung_dir, 0750);
                if (error == -1) {
                        g_message ("Error: Could not create directory %s!",
                                   glosung_dir);
                }
        }
} /* init */


const gchar*
get_last_error_message ()
{
	if (error == 0) {
		return "";
	}
	if (! error_messages) {
		init_error_messages ();
	}
	const gchar* msg = (const gchar*)
		g_hash_table_lookup (error_messages, &error);
	if (! msg) {
		// TODO: CURLOPT_ERRORBUFFER
		return "";
	}
	return msg;
} /* get_last_error_string */


static void
init_error_messages ()
{
	error_messages = g_hash_table_new (g_int_hash, g_int_equal);
	int *idcopy = (int *) g_malloc (sizeof (int));
	*idcopy = CURLE_COULDNT_RESOLVE_PROXY;
	g_hash_table_insert (error_messages, idcopy, _("Couldn't resolve proxy"));
} /* init_error_messages */
