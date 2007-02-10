/* losung.c
 * Copyright (C) 2006-2007 Eicke Godehardt

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

#include "parser.h"

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include <gconf/gconf-client.h>

#include <glib/gi18n.h>
#include <glib/goption.h>

#define PACKAGE "glosung"
#define APPNAME "losung"


/****************************\
   Variables & Definitions
\****************************/

static GDate     *date;

/****************************\
      Function prototypes
\****************************/

static void       get_time              (void);
static void       show_text             (void);

gchar *lang;
gchar *date_param = NULL;


static GOptionEntry entries[] = 
{
  { "language", 'l', 0, G_OPTION_ARG_STRING, &lang, "language of watch word", "de" },
  { "date", 0, 0, G_OPTION_ARG_STRING, &date_param, "date to show watch word for", "2007-02-06" },
  { NULL }
};


/*
 * the one and only main function :-)
 */
int 
main (int argc, char **argv)
{
        /* Initialize the i18n stuff */
        /*
        bindtextdomain (PACKAGE, "/usr/share/locale");
        textdomain (PACKAGE);
        bind_textdomain_codeset (PACKAGE, "UTF-8");
        */

        GError *error = NULL;
        GOptionContext *context;

        GConfClient *client = gconf_client_get_default ();
        lang = gconf_client_get_string
                (client, "/apps/" PACKAGE "/language", NULL);
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
                date = g_date_new ();
                g_date_set_parse (date, date_param);
                if (g_date_valid (date)) {
                        return;
                }
        }

        time_t     t;
        struct tm  zeit;

        t = time (NULL);
        zeit = *localtime (&t);
        date = g_date_new_dmy
                (zeit.tm_mday, zeit.tm_mon + 1, zeit.tm_year + 1900);
        if (date_param [0] == '+' || date_param [0] == '-') {
                int offset = 0;
                int result = sscanf (date_param, "%d", &offset);
                if (date_param [strlen (date_param) - 1] == 'm' ||
                    date_param [strlen (date_param) - 1] == 'M')
                {
                        if (offset > 0) {
                                g_date_add_months (date, offset);
                        } else {
                                g_date_subtract_months (date, -offset);
                        }
                } else {
                        if (offset > 0) {
                                g_date_add_days (date, offset);
                        } else {
                                g_date_subtract_days (date, -offset);
                        }
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
        // guint i;

        ww = get_losung (date, lang);
        if (ww == NULL) {
                /*
                i = 0;
                while (languages [i].lang != NULL) {
                        if (0 == strcmp (lang, languages [i].lang)) {
                                text = g_strdup_printf
                                        (_("No %s texts found for %d!"),
                                         languages [i].name,
                                         g_date_get_year (date));
                                break;
                        }
                        i++;
                }
                */
                printf (_("No %s texts found for %d!\n"),
                        lang, g_date_get_year (date));
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
