/* about.c
 * Copyright (C) 2007-2022 Eicke Godehardt

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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n.h>
#include <adwaita.h>

#include "about.h"
#include "util.h"

#define APPNAME "GLosung"


/*
 * callback function that displays the about dialog.
 */
void
about (GtkWidget *app)
{
        const gchar *authors [] = {
                "Eicke Godehardt",
                "Johannes Hausensteiner",
                "Andreas Kemnade",
                "Sebastian Pätzold (rpm packages)",
                NULL
        };
        const gchar *artists [] = {
                "Watchwords: (C) Moravian Church https://www.losungen.de",
                "Bible 2.0: (C) Bible 2.0 Team http://www.bible20.net",
                "Icons: Font Awesome https://fontawesome.com/license",
                NULL
        };
        gchar *translators =
            "Marek Drápal\nNicolas\nEmanuel Feruzi\nMáté Nagy\nEicke Godehardt";

        AdwStyleManager *manager = adw_style_manager_get_default ();

        GdkPaintable *logo;
        if (adw_style_manager_get_dark (manager)) {
                logo = gtk_picture_get_paintable (
                        GTK_PICTURE (gtk_picture_new_for_filename
                                (PACKAGE_PIXMAPS_DIR "/glosung-big-white.png")));
        } else {
                logo = gtk_picture_get_paintable (
                        GTK_PICTURE (gtk_picture_new_for_filename
                                (PACKAGE_PIXMAPS_DIR "/glosung-big.png")));
        }

        gtk_show_about_dialog (GTK_WINDOW (app),
                 "authors", authors,
                 "artists", artists,
                 "comments", _("Gods word for every day"),
                 "copyright", "(C) 1999-2022 Eicke Godehardt",
                 // "logo-icon-name", PACKAGE_PIXMAPS_DIR "/glosung-big.png",
                 "logo", logo,
                 "name", APPNAME,
                 "program-name", APPNAME,
                 "translator-credits", translators,
                 "title", _("About GLosung"),
                 "version", VERSION,
                 "website", "http://www.godehardt.org/losung.html",
                 "website-label", "http://www.godehardt.org/losung.html",
                 NULL);
} /* about */
