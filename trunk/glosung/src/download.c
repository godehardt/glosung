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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>
#include <errno.h>

//#include <curl/types.h>
//#include <curl/easy.h>

#include "download.h"


#define LOSUNGEN_URL "http://www.brueder-unitaet.de/download/Losung_%d_XML.zip"


typedef struct Memory {
        char *memory;
        size_t size;
} Memory;


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


int
download_losungen (guint year)
{
        gchar *url = g_strdup_printf (LOSUNGEN_URL, year);
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

//                analyse (chunk);
        }
        return 0;
}
