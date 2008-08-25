/* download.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <glib/gstrfuncs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>
#include <errno.h>

//#include <curl/types.h>
//#include <curl/easy.h>

#include "download.h"



#define HEADER_VERSION    1
#define COMPRESS_VERSION  1
#define FILENAMELEN      63

#define LOZ_URL "http://www.losung.de/cgi-bin/langpack.pl"
#define LOZ_LIST_URL LOZ_URL "?cmd=list&dlgLang=de"
// #define LOZ_LIST_URL "file://list"


typedef struct Memory {
        char *memory;
        size_t size;
} Memory;


typedef struct _LozHeader
{
  unsigned int size;             // sizeof (LozHeader)
  unsigned int header_version;   // constant
  unsigned int compress_version; // constant
  unsigned int file_size;        // size of original (uncompressed) file
  char file_date_time [15];      // ISO format: yyyymmddHHMMSS (terminating \0)
  char file_name [FILENAMELEN + 1]; // filename, \0 terminated, \0 filled
  unsigned int mem_size;
} LozHeader;


static unsigned char * unloz   (LozHeader *header, unsigned char *data);
static int             to_file (LozHeader *header, unsigned char *data);




static LozHeader *
get_header (Memory *mem)
{
        LozHeader *header = (LozHeader *) (mem->memory);
        if (header->size != sizeof (LozHeader) - sizeof (unsigned int)) {
                printf ("header size differs\n");
                return NULL;
        }
        if (header->header_version != HEADER_VERSION) {
                printf ("header version differs\n");
                return NULL;
        }
        if (header->compress_version != COMPRESS_VERSION) {
                printf ("compress version differs\n");
                return NULL;
        }

        return header;
}


static unsigned char *
get_data (Memory *mem)
{
        return (unsigned char *) mem->memory + sizeof (LozHeader);
}


static void *
myrealloc (void *ptr, size_t size)
{
        /* There might be a realloc() out there that doesn't like reallocing
           NULL pointers, so we take care of it here */
        if (ptr) {
                return realloc (ptr, size);
        } else {
                return malloc (size);
        }
}


static size_t
WriteMemoryCallback (void *ptr, size_t size, size_t nmemb, void *data)
{
        size_t realsize = size * nmemb;
        Memory *mem = (Memory *)data;

        mem->memory = (char *)myrealloc (mem->memory,
                                         mem->size + realsize + 1);
        if (mem->memory) {
                memcpy (& (mem->memory [mem->size]), ptr, realsize);
                mem->size += realsize;
                mem->memory [mem->size] = 0;
        }
        return realsize;
}


static LosungList*
analyse (Memory chunk)
{
        LosungList* list = losunglist_new ();
        int    start;
        int    end = -1;
        gint   year;
        gchar *lang;

        do {
                end++;
                start = end;
                while (chunk.memory [end] != '\n'
                       && chunk.memory [end] != '\0')
                {
                        end++;
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
}


LosungList*
download_losung_list (void)
{
        CURL     *curl_handle;
        CURLcode  res;
        Memory    chunk;
        LosungList* list = NULL;


        chunk.memory = NULL; /* we expect realloc(NULL, size) to work */
        chunk.size   = 0;    /* no data at this point */

        curl_global_init (CURL_GLOBAL_ALL);
        /* init the curl session */
        curl_handle = curl_easy_init ();
        if (curl_handle) {
                /* specify URL to get */
                curl_easy_setopt (curl_handle, CURLOPT_URL, LOZ_LIST_URL);

                /* send all data to this function  */
                curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION,
                                  WriteMemoryCallback);

                /* we pass our 'chunk' struct to the callback function */
                curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA,
                                  (void *)&chunk);

                /* some servers don't like requests that are made
                   without a user-agent field, so we provide one */
                curl_easy_setopt (curl_handle, CURLOPT_USERAGENT,
                                  "glosung/3.0");

                /* get it! */
                res = curl_easy_perform (curl_handle);

                /* cleanup curl stuff */
                curl_easy_cleanup (curl_handle);

                list = analyse (chunk);
        }
        return list;
}


int
download_losung (gchar *lang, guint year)
{
        gchar *url = g_strdup_printf
                (LOZ_URL "?cmd=get&dlgLang=en&docLang=%s&year=%d", lang, year);
        CURL     *curl_handle;
        CURLcode  res;
        Memory    chunk;

        chunk.memory = NULL; /* we expect realloc (NULL, size) to work */
        chunk.size   = 0;    /* no data at this point */

        curl_global_init (CURL_GLOBAL_ALL);
        /* init the curl session */
        curl_handle = curl_easy_init ();
        if (curl_handle) {
                /* specify URL to get */
                curl_easy_setopt (curl_handle, CURLOPT_URL, url);

                /* send all data to this function  */
                curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION,
                                  WriteMemoryCallback);

                /* we pass our 'chunk' struct to the callback function */
                curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA,
                                  (void *)&chunk);


                /* some servers don't like requests that are made
                   without a user-agent field, so we provide one */
                curl_easy_setopt (curl_handle, CURLOPT_USERAGENT,
                                  "glosung/" VERSION);

                /* get it! */
                res = curl_easy_perform (curl_handle);

                /* cleanup curl stuff */
                curl_easy_cleanup (curl_handle);

                analyse (chunk);
        }
        return 0;
}











static unsigned char
compute_xor_seed (const char *filename)
{
        unsigned int sum = 0;
        const char *p;
        for (p = filename; *p; ++p) {
                sum += *p;
        }
        if (sum) {
                return (unsigned char)(sum & 0xff);
        } else {
                return 0xff;
        }
}


static int
to_file (LozHeader *header, unsigned char *data)
{
        gchar *losungdir = g_strdup_printf
                ("%s/.glosung", getenv ("HOME"));
        struct stat buf;
        if (stat (losungdir, &buf) == -1 && errno == ENOENT) {
                int error = mkdir (losungdir, 0750);
                if (error == -1) {
                        g_message ("Error: Could not create directory %s!",
                                   losungdir);
                }
        }
        gchar *filename = g_strdup_printf
                ("%s/%s", losungdir, header->file_name);
        /* g_message (filename); */

        FILE *file = fopen (filename, "wb");
        if (file) {
                size_t written = fwrite (data, 1, header->file_size, file);
                // printf ("written: %d\n", written);
                if (written == 1) {
                        // result = 0;
                }
                fclose (file);
        }

        g_free (filename);
        return 0;
}


static unsigned char *
unloz (LozHeader *header, unsigned char *data)
{
        unsigned char xor_seed = compute_xor_seed (header->file_name);
        unsigned char *p;

        for (p = data; p < data + header->mem_size; ++p) {
                *p ^= xor_seed;
        }

        unsigned char *dest_data  = malloc (sizeof (char) * header->file_size);
        unsigned long uncompr_len = header->file_size;
        int err = uncompress (dest_data, &uncompr_len, data, header->mem_size);
        if (err) {              /* FIXME: check error conditions */
                printf ("%d\n", err);
                return NULL;
        }

        return dest_data;
}
