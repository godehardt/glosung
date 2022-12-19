/* losung.c
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

#define ENABLE_NLS
#define PACKAGE "glosung"

#include "parser.h"

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include <glib/gi18n.h>
#include <glib.h>

#include "util.h"
#include "settings.h"


#define PACKAGE "glosung"


/****************************\
   Variables & Definitions
\****************************/

static GDateTime *date;
static gchar     *lang = NULL;
static gchar     *date_param = NULL;


/****************************\
      Function prototypes
\****************************/

static void get_time    (void);
static void show_text   (void);


static GOptionEntry entries[] = {
  { "language", 'l', 0, G_OPTION_ARG_STRING, &lang,
    "language of watch word", "de" },
  { "date", 0, 0, G_OPTION_ARG_STRING, &date_param,
    "set date absolute or relative, e.g., +1, -2m", "2008-08-05" },
  { NULL }
};


/*
 * the one and only main function :-)
 */
int 
main (int argc, char **argv)
{
        GError *error = NULL;
        GOptionContext *context;

        bindtextdomain (PACKAGE, "/usr/share/locale");
        bind_textdomain_codeset (PACKAGE, "UTF-8");
        textdomain (PACKAGE);

        lang = get_language ();
        if (lang == NULL) {
                lang = "de";
        }

        context = g_option_context_new ("- watch word program");
        g_option_context_add_main_entries (context, entries, NULL);
        g_option_context_parse (context, &argc, &argv, &error);

        get_time ();
        show_text ();

        return 0;
} /* main */


/*
 * computes local date and fill GDate date.
 */
static void 
get_time (void)
{
        if (date_param != NULL) {
                date = g_date_time_new_from_iso8601 (date_param, NULL);
                if (date != NULL) {
                        return;
                }
        }

        date = g_date_time_new_now_local ();
        if (date_param != NULL &&
            (date_param [0] == '+' || date_param [0] == '-'))
        {
                int offset = 0;
                sscanf (date_param, "%d", &offset);
                if (date_param [strlen (date_param) - 1] == 'm' ||
                    date_param [strlen (date_param) - 1] == 'M')
                {
                        date = g_date_time_add_months (date, offset);
                } else {
                        date = g_date_time_add_days (date, offset);
                }
        }
} /* get_time */


/*
 * display the ww for the current date.
 */
static void 
show_text (void)
{
        const Losung *ww;

        ww = get_orig_losung (date, lang);
        if (! ww) {
                ww = get_losung (date, lang);
                if (! ww) {
                        ww = get_the_word (date, lang);
                }
        } else {
        	wrap_text (ww->ot.text, 50);
        	wrap_text (ww->nt.text, 50);
        }

        if (ww == NULL) {
                printf (_("No '%s' texts found for %d!\n"),
                        lang, g_date_time_get_year (date));
                return;
        }

        printf ("%s\n\n", ww->title);
        if (ww->ot.say != NULL) {
                printf ("%s\n", ww->ot.say);
        }
        printf ("%s\n", ww->ot.text);
        printf ("%30s\n\n", ww->ot.location);

        if (ww->nt.say != NULL) {
                printf ("%s\n", ww->nt.say);
        }
        printf ("%s\n", ww->nt.text);
        printf ("%30s\n", ww->nt.location);
        
        /*
        if (ww->comment) {
                GtkWidget *comment;

                comment = gtk_message_dialog_new (
                        GTK_WINDOW (app), GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, ww->comment);

                g_signal_connect (G_OBJECT (comment), "response",
                                  G_CALLBACK (gtk_widget_destroy), NULL);

                gtk_widget_show (comment);
        }

        if (ww->selective_reading && ww->continuing_reading) {
                gchar *text;

                text = g_strconcat (ww->selective_reading, " :: ",
                                    ww->continuing_reading, NULL);
                gtk_label_set_text  (GTK_LABEL (label [READING]), text);
                g_free (text);
        }

        */
        losung_free (ww);
} /* show_text */
