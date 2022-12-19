/* util.c
 * Copyright (C) 2009-2022 Eicke Godehardt

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

#include <string.h>

#include "util.h"


static gchar* file_exist (gchar* path, gchar* filename);


void
wrap_text (gchar *string, gint width)
{
        int len = strlen (string);
        int index = 0;

        while (index + width < len) {
                index += width;
                while (string [index] != ' ') {
                        index--;
                }
                string [index] = '\n';
        }
} /* wrap_text */


gchar*
find_ui_file (gchar* filename)
{
	gchar* file;

	if ((file = file_exist ("ui/", filename)) != NULL) {
		return file;
	} else if ((file = file_exist (PACKAGE_PIXMAPS_DIR, filename)) != NULL) {
		return file;
	}

	return NULL;
} /* find_ui_file */


static gchar*
file_exist (gchar* path, gchar* filename)
{
	gchar* file = g_strconcat (path, filename, NULL);
	if (g_file_test (file, G_FILE_TEST_EXISTS)) {
		return file;
	}
	g_free (file);

	return NULL;
} /* file_exist */


/*
 * callback function to open a specific uri (xiphos or http)
 */
void
show_uri (GtkWidget *window, gchar *uri, gpointer data)
{
	/* FIXME: handle return in 
	gtk_show_uri_full (GTK_WINDOW (window), (const char*) uri, GDK_CURRENT_TIME, NULL, callback, NULL);
	if (result) {
		return;
	}
	*/

	gtk_show_uri (GTK_WINDOW (window), (const char*) uri, GDK_CURRENT_TIME);


	/* this is only executed for older gtk+ or if gtk_show_uri failed */
	if (strncmp (uri, "http://", 7) == 0) {
		char *argv [3];
		argv [0] = "xdg-open";
		argv [1] = (char *) uri;
		argv [2] = NULL;

		GError *error = NULL;
		if (! g_spawn_async (NULL, argv, NULL,
					G_SPAWN_STDOUT_TO_DEV_NULL
					| G_SPAWN_STDERR_TO_DEV_NULL
					| G_SPAWN_SEARCH_PATH,
					NULL, NULL, NULL, &error)) {
			GtkWidget *msg = gtk_message_dialog_new
					(NULL, GTK_DIALOG_MODAL,
						GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
						"%s", error->message);
			g_signal_connect (G_OBJECT (msg), "response",
								G_CALLBACK (gtk_window_destroy), NULL);
			gtk_widget_show (msg);
			g_error_free (error);
		}
			return;
	} else {
		char *argv [3];
		argv [0] = "xiphos";
		argv [1] = uri;
		argv [2] = NULL;

		GError *error = NULL;
		if (! g_spawn_async (NULL, argv, NULL,
					G_SPAWN_STDOUT_TO_DEV_NULL
				| G_SPAWN_STDERR_TO_DEV_NULL
				| G_SPAWN_SEARCH_PATH,
					NULL, NULL, NULL, &error))
		{
			argv [0] = "gnomesword2";
			if (! g_spawn_async (NULL, argv, NULL,
					G_SPAWN_STDOUT_TO_DEV_NULL
				| G_SPAWN_STDERR_TO_DEV_NULL
				| G_SPAWN_SEARCH_PATH,
					NULL, NULL, NULL, &error))
			{
				GtkWidget *msg = gtk_message_dialog_new
					(GTK_WINDOW (data),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
						"%s", error->message);
				//_("No text files found!\n");
				g_signal_connect
					(G_OBJECT (msg), "response",
						G_CALLBACK (gtk_window_destroy),NULL);
				gtk_widget_show (msg);
				g_error_free (error);
			}
		}
	}
} /* show_uri */
