/* download.c
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

//#include <curl/types.h>
//#include <curl/easy.h>

#include "download.h"
#include "losunglist.h"

#define LOSUNGEN_URL "http://www.brueder-unitaet.de/download/Losung_%d_XML.zip"
#define BIBLE20_BASE_URL "http://bible20.net/service/TheWord/twd11/"


static gchar *glosung_dir = NULL;

typedef struct Memory {
        char *memory;
        size_t size;
} Memory;


static void   init     ();
static int    to_file_losungen  (Memory chunk);
static Memory download (gchar *url);



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


static int
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
                        return -2;
                }
        }
        return 0;
}


static int
to_file_losungen (Memory chunk)
{
        init ();

        gchar *tmp_dir = g_strdup_printf ("%s/tmp_download", glosung_dir);
        gchar *zipfile = g_strdup_printf ("%s/Losung_XML.zip", tmp_dir);

#ifdef WIN32
        mkdir (tmp_dir);
#else /* WIN32 */
        mkdir (tmp_dir, 0777);
#endif /* WIN32 */
        int res = to_file (chunk, zipfile);
        if (res) {
                rmdir (tmp_dir);
                g_free (tmp_dir);
                g_free (zipfile);
                return res;
        }

        // TODO: use getcwd (char *buf, size_t size); and reset afterwards
        if (chdir (tmp_dir) != 0) {
                return -4;
        }
        gchar *command = g_strdup_printf ("unzip %s", zipfile);
        gboolean success =
                g_spawn_command_line_sync (command, NULL, NULL, NULL, NULL);

        GDir *dir = g_dir_open (tmp_dir, 0, NULL);
        const gchar *name;
        while ((name = g_dir_read_name (dir))) {
                if ((strncmp (name + strlen (name) -  4, ".xml", 4)) == 0) {
                        gchar *losung_file =
                                g_strdup_printf ("%s/%s", tmp_dir, name);
                        gchar *target_file =
                                g_strdup_printf ("%s/%s", glosung_dir, name);
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
        g_free (zipfile);
        g_free (command);
        if (! success) {
                g_message ("error while unzip");
                return -3;
        }

        return 0;
}


int
download_losungen (guint year)
{
        gchar *url   = g_strdup_printf (LOSUNGEN_URL, year);
        Memory chunk = download (url);

        to_file_losungen (chunk);
        return 0;
}


/*
 * generic method to download specified url to chunk of memory
 */
static Memory
download (gchar *url)
{
        CURL     *curl_handle;
        CURLcode  res = -1;
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

                res = curl_easy_perform (curl_handle);
                curl_easy_cleanup (curl_handle);

                /*
                if (res == CURLE_OK) {
                        return chunk;
                }
                */
        }
        return chunk;
}


static void
analyse_bible20_list (Memory mem)
{
        LosungList* list = losunglist_new ();
        gchar** lines = g_strsplit (mem.memory, "\n", -1);
        GHashTable *columns = g_hash_table_new (g_str_hash, g_str_equal);
        gint i = 0;
        while (lines [i]) {
                g_message ("%s", lines [i]);
                gchar** tokens = g_strsplit (lines [i], "\";", -1);
        }
        /*
                i = 0;
                while (tokens [i]) {
                        g_hash_table_insert (columns, tokens [i], GINT_TO_POINTER (i));
                }

                switch (chunk.memory [start]) {
                case '#':
                        break;
                case 'F':
                        if (isdigit (chunk.memory [start + 1]) &&
                            isdigit (chunk.memory [start + 2]) &&
                            isdigit (chunk.memory [start + 3]) &&
                            isdigit (chunk.memory [start + 4]))
                        {
                                year =  (chunk.memory [++start] - 48) * 1000;
                                year += (chunk.memory [++start] - 48) * 100;
                                year += (chunk.memory [++start] - 48) * 10;
                                year += (chunk.memory [++start] - 48);
                                start += 2;
                                lang = g_strndup (chunk.memory + start,
                                                  end - start - 1);
                                losunglist_add (list, lang, year);
                        }
                        break;
                case 'D':
                        chunk.memory += start + 1;
                        chunk.size -= start + 1;

                        LozHeader     *header = get_header (& chunk);
                        unsigned char *data   = get_data   (& chunk);
                        data = unloz (header, data);
                        to_file (header, data);
                        return NULL;

                        break;
                }
        } while (chunk.memory [end] != '\0');

        losunglist_finialize (list);
        return list;
        */
}


static void
init (void)
{
        if (glosung_dir) {
                return; /* config_path already set */
        }

        glosung_dir = g_strdup_printf
                ("%s/.glosung", getenv ("HOME"));
        if (! g_file_test (glosung_dir, G_FILE_TEST_IS_DIR)) {
#ifdef WIN32
                int error = mkdir (glosung_dir);
#else /* WIN32 */
                int error = mkdir (glosung_dir, 0750);
#endif /* WIN32 */
                if (error == -1) {
                        g_message ("Error: Could not create directory %s!",
                                   glosung_dir);
                }
        }
}
