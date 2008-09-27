/* about.c
 * Copyright (C) 2007-2008 Eicke Godehardt

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

#include <glib.h>
#include <glib/gi18n.h>

#include "about.h"

#define APPNAME "GLosung"


/*
 * callback function that displays the about dialog.
 */
void 
about (GtkWidget *app)
{
        const gchar *authors [] = {
                "Eicke Godehardt",
                "Sebastian Pätzold (rpm packages)",
                NULL
        };
        gchar *translators =
                "Marek Drápal\nEicke Godehardt\nNicolas\nEmanuel Feruzi";

        GError *error = NULL;
        GdkPixbuf *logo =  gdk_pixbuf_new_from_file
                (PACKAGE_PIXMAPS_DIR "/glosung-big.png", &error);

        gtk_show_about_dialog (GTK_WINDOW (app),
                 "authors", authors,
                 "comments", _("Gods word for every day"),
                 "copyright", "(C) 1999-2008 Eicke Godehardt",
                 // "logo-icon-name", PACKAGE_PIXMAPS_DIR "/glosung-big.png",
                 "logo", logo,
                 "name", APPNAME,
                 "translator-credits", translators,
                 "version", VERSION,
                 "website", "http://www.godehardt.org/losung.html",
                 NULL);
} /* about */
