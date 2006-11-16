/* glosung.c
 * Copyright (C) 1999-2006 Eicke Godehardt

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
/* $Id:$ */


#include <string.h>
#include <time.h>
#include <unistd.h>

#include <gconf/gconf-client.h>

#include <glib/gi18n.h>

#include <gtk/gtkaboutdialog.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcalendar.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkfontbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktable.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkvbox.h>

#include <libgnome/gnome-program.h>
#include <libgnomeui/gnome-app.h>
#include <libgnomeui/gnome-app-helper.h>
#include <libgnomeui/gnome-href.h>
#include <libgnomeui/gnome-ui-init.h>
#include <libgnomeui/gnome-uidefs.h>

#include "parser.h"
#include "download.h"


#define ENABLE_NLS

#define PACKAGE "glosung"
#define APPNAME "GLosung"


/****************************\
   Variables & Definitions
\****************************/

enum {
        TITLE,
        X1,
        OT_TEXT,
        OT_LOC,
        OT_LOC_SWORD,
        X2,
        NT_TEXT,
        NT_LOC,
        NT_LOC_SWORD,
        X3,
        READING,
        NUMBER_OF_LABELS
};

static GConfClient *client;
static GtkWidget *app;
static GDate     *date;
static GtkWidget *calendar;
static GtkWidget *label [NUMBER_OF_LABELS];
static GtkWidget *property = NULL;

static gchar     *new_lang = NULL;
static gchar     *font = NULL;
static gchar     *new_font;
static gboolean   calendar_close;
static gboolean   calendar_close_new;
static gboolean   show_readings;
static gboolean   show_readings_new;
static gboolean   show_sword;
static gboolean   show_sword_new;

static struct {
        const gchar *lang;
        guint        length;    /* length of lang */
        const gchar *name;
        gboolean     found;
} languages [] = {
        {"de",    2, N_("german"),  FALSE},
        {"cs",    2, N_("czech"),   FALSE},
        {"en",    2, N_("english"), FALSE},
        {"es",    2, N_("spanish"), FALSE},
        {"fr",    2, N_("french"), FALSE},
        {"zh-CN", 5, N_("simplified chinese"),  FALSE},
        {"zh-TW", 5, N_("traditional chinese"), FALSE},
        {NULL, -1, NULL, FALSE}
};
static guint langs;

gchar          *lang;


/* static GnomeHelpMenuEntry help_ref = { "glosung", "pbox.html" }; */

/****************************\
      Function prototypes
\****************************/

static void scan_for_languages          (void);

static void       get_time              (void);
static void       show_text             (void);
static void       create_app            (void);
static GtkWidget *create_property_table (void);

static void about_cb             (GtkWidget *w, gpointer data);
static void apply_cb             (void);
static void calendar_cb          (GtkWidget *w, gpointer data);
static void calendar_option_cb   (GtkWidget *toggle,   gpointer data);
static void calendar_select_cb   (GtkWidget *calendar, gpointer data);
static void exit_cb              (GtkWidget *w, gpointer data);
static void font_sel_cb          (GtkWidget *button,   gpointer data);
static void lang_changed_cb      (GtkWidget *item,     gpointer data);
static void lang_manager_cb      (GtkWidget *w, gpointer data);
static void next_day_cb          (GtkWidget *w, gpointer data);
static void next_month_cb        (GtkWidget *w, gpointer data);
static void prev_day_cb          (GtkWidget *w, gpointer data);
static void prev_month_cb        (GtkWidget *w, gpointer data);
static void property_cb          (GtkWidget *w, gpointer data);
static void property_response_cb (GtkDialog *dialog,
                                  gint       arg1,
                                  gpointer   user_data);
static void readings_cb          (GtkWidget *toggle,   gpointer data);
static void sword_cb             (GtkWidget *toggle,   gpointer data);
static void today_cb             (GtkWidget *w, gpointer data);


/******************************\
       Set up the GUI
\******************************/

static GnomeUIInfo losung_menu[] = {
        GNOMEUIINFO_ITEM_NONE (N_("Calendar"),
                               N_("Open a calendar-widget"),
                               calendar_cb),
        GNOMEUIINFO_ITEM_STOCK (N_("Today"),
                                N_("Show losung of today"),
                                today_cb, GTK_STOCK_REFRESH),
        GNOMEUIINFO_ITEM_STOCK (N_("Next Day"),
                                N_("Show losung of next day"),
                                next_day_cb, GTK_STOCK_GO_FORWARD),
        GNOMEUIINFO_ITEM_STOCK (N_("Prev Day"),
                                N_("Show losung of previous day"),
                                prev_day_cb, GTK_STOCK_GO_BACK),
        GNOMEUIINFO_ITEM_STOCK (N_("Next Month"),
                                N_("Show losung of next month"),
                                next_month_cb, GTK_STOCK_GOTO_LAST),
        GNOMEUIINFO_ITEM_STOCK (N_("Prev Month"),
                                N_("Show losung of previous month"),
                                prev_month_cb, GTK_STOCK_GOTO_FIRST),
        GNOMEUIINFO_SEPARATOR,
        GNOMEUIINFO_MENU_EXIT_ITEM (exit_cb, NULL),
        GNOMEUIINFO_END
}; /* losung_menu */

static GnomeUIInfo settings_menu[] = {
        GNOMEUIINFO_ITEM_STOCK (N_("Preferences..."),
                                N_("To edit the preferences"),
                                property_cb, GTK_STOCK_PREFERENCES),
        GNOMEUIINFO_ITEM_NONE (N_("Languages..."),
                                N_("To update/install language files"),
                                lang_manager_cb),
        GNOMEUIINFO_END
}; /* settings_menu */

static GnomeUIInfo help_menu[] = {
        /* GNOMEUIINFO_HELP ("glosung"), */
        GNOMEUIINFO_MENU_ABOUT_ITEM (about_cb, NULL),
        GNOMEUIINFO_END
}; /* help_menu */

static GnomeUIInfo main_menu[] = {
        GNOMEUIINFO_SUBTREE            (N_("_Losung"), losung_menu),
        GNOMEUIINFO_MENU_SETTINGS_TREE (settings_menu),
        GNOMEUIINFO_MENU_HELP_TREE     (help_menu),
        GNOMEUIINFO_END
}; /* main_menu */

static GnomeUIInfo toolbar[] = {
        GNOMEUIINFO_ITEM_STOCK (N_("Exit"),
                                N_("Exit the Program"), 
                                exit_cb, GTK_STOCK_QUIT),
        GNOMEUIINFO_SEPARATOR,
        GNOMEUIINFO_ITEM_STOCK (N_("Prev Day"),
                                N_("Show losung of previous day"),
                                prev_day_cb, GTK_STOCK_GO_BACK),
        GNOMEUIINFO_ITEM_STOCK (N_("Today"),
                                N_("Show losung of today"),
                                today_cb, GTK_STOCK_REFRESH),
        GNOMEUIINFO_ITEM_STOCK (N_("Next Day"),
                                N_("Show losung of next day"),
                                next_day_cb, GTK_STOCK_GO_FORWARD),
        GNOMEUIINFO_END
}; /* toolbar */


/*
 * the one and only main function :-)
 */
int 
main (int argc, char **argv)
{
        /* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, "/usr/share/locale");
        textdomain (PACKAGE);
        bind_textdomain_codeset (PACKAGE, "UTF-8");

        /* Initialize gnome and argument stuff */
        gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
                            argc, argv, GNOME_PARAM_NONE);
        gtk_window_set_default_icon_from_file
                (PACKAGE_PIXMAPS_DIR "/glosung.png", NULL);
        client = gconf_client_get_default ();

        scan_for_languages ();
        lang = gconf_client_get_string
                (client, "/apps/" PACKAGE "/language", NULL);
        if (lang == NULL && langs > 0) {
                guint i;
                /* should be translated to corresponding language, e.g. 'de' */
                lang = _("en");

                /* is requested language available,
                   if not use first available language instead */
                i = 0;
                while (languages [i].lang != NULL) {
                        if (0 == strcmp (lang, languages [i].lang)) {
                                break;
                        }
                        i++;
                }
                if (languages [i].lang == NULL) {
                        i = 0;
                        while (languages [i].lang != NULL) {
                                if (languages [i].found == TRUE) {
                                        lang = (gchar*)languages [i].lang;
                                        break;
                                }
                                i++;
                        }
                }
        }
        calendar_close = calendar_close_new = gconf_client_get_bool
                (client, "/apps/" PACKAGE "/calendar_close_by_double_click",
                 NULL);
        show_readings = show_readings_new = gconf_client_get_bool
                (client, "/apps/" PACKAGE "/show_readings", NULL);
        show_sword = show_sword_new = gconf_client_get_bool
                (client, "/apps/" PACKAGE "/link_sword", NULL);
        font = gconf_client_get_string (client, "/apps/" PACKAGE "/font",
                                        NULL);

        create_app ();
        if (langs == 0) {
                GtkWidget *error = gtk_message_dialog_new
                        (GTK_WINDOW (app), GTK_DIALOG_DESTROY_WITH_PARENT,
                         GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                         _("No text files found!\n"
                           "Please contact your administrator."));
                g_signal_connect (G_OBJECT (error), "response",
                                  G_CALLBACK (exit_cb), NULL);
                gtk_widget_show (error);
        } else {
                get_time ();
                show_text ();
        }

        gtk_main ();

        /* exit (EXIT_SUCCESS); */
        return 0;
} /* main */


static void
scan_for_languages (void) 
{
        guint i;
        langs = 0;
        if (access (GLOSUNG_DATA_DIR, F_OK | R_OK) != 0) {
                return;
        }
        GDir  *dir = g_dir_open (GLOSUNG_DATA_DIR, 0, NULL);
        const  gchar *name;

        while ((name = g_dir_read_name (dir)) != NULL) {
                i = 0;
                while (languages [i].lang != NULL) {
                        if ((0 == strncmp (name, languages [i].lang,
                                           languages [i].length))
                            && (0 == strncmp (name + languages [i].length,
                                              "_los", 4))
                            && (0 == strncmp (name + strlen (name) - 4,
                                              ".xml", 4))) {
                                languages [i].found = TRUE;
                                langs++;
                                break;
                        }
                        i++;
                }
        }
        i = 0;
        while (languages [i].lang != NULL) {
                if (languages [i].found) {
                        g_message ("found language %s",
                                   languages [i].name);
                }
                i++;
        }
} /* scan_for_languages */


/*
 * does all the GUI stuff.
 */
static void
create_app (void)
{
        GtkWidget *vbox;
        gint       i;
        PangoFontDescription *font_desc;

        app = gnome_app_new (PACKAGE, APPNAME);
        g_signal_connect (G_OBJECT (app), "delete_event",
                          G_CALLBACK (exit_cb), NULL);

        gnome_app_create_menus_with_data   (GNOME_APP (app), main_menu, app);
        gnome_app_create_toolbar_with_data (GNOME_APP (app), toolbar, app);

        vbox = gtk_vbox_new (FALSE, 0);
        gnome_app_set_contents (GNOME_APP (app), vbox);
        gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD);

        for (i = 0; i < NUMBER_OF_LABELS; i++) {
                if (i == OT_LOC_SWORD || i == NT_LOC_SWORD) {
                        GtkWidget *widget;
                        widget = gtk_hbutton_box_new ();
                        gtk_button_box_set_layout
                                (GTK_BUTTON_BOX (widget),
                                 GTK_BUTTONBOX_SPREAD);
                        label [i] = gnome_href_new ("", "");
                        gtk_container_add (GTK_CONTAINER (widget), label [i]);
                        gtk_widget_show (label [i]);

                        gtk_box_pack_start (GTK_BOX (vbox), widget,
                                            FALSE, FALSE, 0);
                        gtk_widget_show (widget);
                } else {
                        label [i] = gtk_label_new ("");
                        gtk_label_set_justify (GTK_LABEL (label [i]),
                                               GTK_JUSTIFY_CENTER);
                        gtk_box_pack_start (GTK_BOX (vbox), label [i],
                                            FALSE, FALSE, 0);
                        gtk_widget_show (label [i]);
                }
        }
        if (font != NULL) {
                font_desc = pango_font_description_from_string (font);
                for (i = 0; i < NUMBER_OF_LABELS; i++) {
                        gtk_widget_modify_font (label [i], font_desc);
                }
                pango_font_description_free (font_desc);
        }
        if (!show_readings) {
                gtk_widget_hide (label [X3]);
                gtk_widget_hide (label [READING]);
        }
        if (show_sword) {
                gtk_widget_hide (label [OT_LOC]);
                gtk_widget_show (label [OT_LOC_SWORD]);
                gtk_widget_hide (label [NT_LOC]);
                gtk_widget_show (label [NT_LOC_SWORD]);
        } else {
                gtk_widget_hide (label [OT_LOC_SWORD]);
                gtk_widget_show (label [OT_LOC]);
                gtk_widget_hide (label [NT_LOC_SWORD]);
                gtk_widget_show (label [NT_LOC]);
        }
        gtk_widget_show (vbox);
        gtk_widget_show (app);
} /* create_app */


/*
 * computes local date and fill GDate date.
 */
static void 
get_time (void)
{
        time_t     t;
        struct tm  zeit;

        t = time (NULL);
        zeit = *localtime (&t);
        date = g_date_new_dmy
                (zeit.tm_mday, zeit.tm_mon + 1, zeit.tm_year + 1900);
} /* get_time */


/*
 * display the ww for the current date.
 */
static void 
show_text (void)
{
        const Losung *ww;
        guint i;

        ww = get_losung (date, lang);
        if (ww == NULL) {
                GtkWidget *error;
                gchar *text = NULL;

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
                error = gtk_message_dialog_new (
                        GTK_WINDOW (app), GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, text);
                g_signal_connect (G_OBJECT (error), "response",
                                  G_CALLBACK (gtk_widget_destroy), NULL);
                gtk_widget_show (error);
                g_free (text);
                return;
        }

        gtk_label_set_text (GTK_LABEL (label [TITLE]), ww->title);
        if (ww->ot.say != NULL) {
                gchar *text;

                text = g_strconcat (ww->ot.say, "\n", ww->ot.text, NULL);
                gtk_label_set_text (GTK_LABEL (label [OT_TEXT]), text);
                g_free (text);
        } else {
                gtk_label_set_text (GTK_LABEL (label [OT_TEXT]), ww->ot.text);
        }
        gnome_href_set_text (GNOME_HREF (label [OT_LOC_SWORD]),
                             ww->ot.location);
        gnome_href_set_url  (GNOME_HREF (label [OT_LOC_SWORD]),
                             ww->ot.location_sword);
        gtk_label_set_text (GTK_LABEL (label [OT_LOC]), ww->ot.location);

        if (ww->nt.say != NULL) {
                gchar *text;
                
                text = g_strconcat (ww->nt.say, "\n", ww->nt.text, NULL);
                gtk_label_set_text (GTK_LABEL (label [NT_TEXT]), text);
                g_free (text);
        } else {
                gtk_label_set_text (GTK_LABEL (label [NT_TEXT]), ww->nt.text);
        }
        gnome_href_set_text (GNOME_HREF (label [NT_LOC_SWORD]),
                             ww->nt.location);
        gnome_href_set_url  (GNOME_HREF (label [NT_LOC_SWORD]),
                             ww->nt.location_sword);
        gtk_label_set_text (GTK_LABEL (label [NT_LOC]), ww->nt.location);

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

        for (i = 0; i < NUMBER_OF_LABELS; i++) {
                if (i != OT_LOC_SWORD && i != NT_LOC_SWORD) {
                        gtk_label_set_use_markup (GTK_LABEL (label [i]), TRUE);
                }
        }
        losung_free (ww);
} /* show_text */


/*
 * callback function that displays the about dialog.
 */
static void 
about_cb (GtkWidget *w, gpointer data)
{
        const gchar *authors [] = {
                "Eicke Godehardt",
                NULL
        };
        gchar *translators = "Nikolas\nMarek Drápal\nEicke Godehardt";

        GError *error = NULL;
        GdkPixbuf *logo =  gdk_pixbuf_new_from_file
                (PACKAGE_PIXMAPS_DIR "/glosung-big.png", &error);

        gtk_show_about_dialog (GTK_WINDOW (app),
                 "authors", authors,
                 "comments", _("Gods word for every day"),
                 "copyright", "(C) 1999-2006 Eicke Godehardt",
                 // "logo-icon-name", PACKAGE_PIXMAPS_DIR "/glosung-big.png",
                 "logo", logo,
                 "name", APPNAME,
                 "translator-credits", translators,
                 "version", VERSION,
                 "website", "http://www.godehardt.org/losung.html",
                 NULL);
} /* about_cb */


/*
 * callback function that displays a property dialog.
 */
static void 
property_cb (GtkWidget *w, gpointer data)
{
        if (property != NULL) {
                g_assert (GTK_WIDGET_REALIZED (property));
                gdk_window_show  (property->window);
                gdk_window_raise (property->window);
        } else {
                if (new_font != NULL) {
                        g_free (new_font);
                        new_font = NULL;
                }

                property = gtk_dialog_new ();
                gtk_window_set_title (GTK_WINDOW (property), _("Preferences"));
                /*
                gtk_dialog_add_action_widget (
                        GTK_DIALOG (property),
                        gtk_button_new_from_stock (GTK_STOCK_HELP),
                        GTK_RESPONSE_HELP);
                */
                gtk_dialog_add_action_widget (
                        GTK_DIALOG (property),
                        gtk_button_new_from_stock (GTK_STOCK_OK),
                        GTK_RESPONSE_OK);
                gtk_dialog_add_action_widget (
                        GTK_DIALOG (property),
                        gtk_button_new_from_stock (GTK_STOCK_APPLY),
                        GTK_RESPONSE_APPLY);
                gtk_dialog_add_action_widget (
                        GTK_DIALOG (property),
                        gtk_button_new_from_stock (GTK_STOCK_CLOSE),
                        GTK_RESPONSE_CLOSE);
                gtk_dialog_set_response_sensitive (
                        GTK_DIALOG (property), GTK_RESPONSE_APPLY, FALSE);

                gtk_container_add (GTK_CONTAINER (GTK_DIALOG (property)->vbox),
                                   create_property_table ());

                g_signal_connect (G_OBJECT (property), "response",
                                  G_CALLBACK (property_response_cb), NULL);
                g_signal_connect (G_OBJECT (property), "destroy",
                                 G_CALLBACK (gtk_widget_destroyed), &property);

                gtk_widget_show_all (property);
                }
} /* property_cb */


/*
 * callback function for property box.
 */
static void 
property_response_cb (GtkDialog *dialog, gint arg1, gpointer user_data)
{
        switch (arg1) {
        case GTK_RESPONSE_NONE:
        case GTK_RESPONSE_HELP:
        case GTK_RESPONSE_DELETE_EVENT:
                break;
        case GTK_RESPONSE_APPLY:
                apply_cb ();
                break;
        case GTK_RESPONSE_OK:
                apply_cb ();
        case GTK_RESPONSE_CLOSE:
                gtk_widget_destroy (GTK_WIDGET (dialog));
                break;
        }
} /* property_response_cb */


/*
 * callback function for property box.
 */
static void 
apply_cb (void)
{
        if (new_lang != NULL) {
                lang = new_lang;
                new_lang = NULL;
                gconf_client_set_string
                        (client, "/apps/" PACKAGE "/language", lang, NULL);
                show_text ();
        }
        if (new_font != NULL) {
                PangoFontDescription *font_desc;
                gint i;

                if (font != NULL) {
                        g_free (font);
                }
                font = new_font;
                new_font = NULL;
                gconf_client_set_string (client, "/apps/" PACKAGE "/font",
                                         font, NULL);

                font_desc = pango_font_description_from_string (font);
                for (i = 0; i < NUMBER_OF_LABELS; i++) {
                        gtk_widget_modify_font (label [i], font_desc);
                }
                pango_font_description_free (font_desc);
        }
        if (calendar_close_new != calendar_close) {
                calendar_close = calendar_close_new;
                gconf_client_set_bool
                        (client,
                         "/apps/" PACKAGE "/calendar_close_by_double_click",
                         calendar_close, NULL);
        }
        if (show_readings_new != show_readings) {
                show_readings = show_readings_new;
                gconf_client_set_bool (client,
                                       "/apps/" PACKAGE "/show_readings",
                                       show_readings, NULL);
                if (show_readings) {
                        gtk_widget_show (label [X3]);
                        gtk_widget_show (label [READING]);
                } else {
                        gtk_widget_hide (label [X3]);
                        gtk_widget_hide (label [READING]);
                }
        }
        if (show_sword_new != show_sword) {
                show_sword = show_sword_new;
                gconf_client_set_bool (client, "/apps/" PACKAGE "/link_sword",
                                       show_sword, NULL);
                if (show_sword) {
                        gtk_widget_hide (label [OT_LOC]);
                        gtk_widget_show (label [OT_LOC_SWORD]);
                        gtk_widget_hide (label [NT_LOC]);
                        gtk_widget_show (label [NT_LOC_SWORD]);
                } else {
                        gtk_widget_hide (label [OT_LOC_SWORD]);
                        gtk_widget_show (label [OT_LOC]);
                        gtk_widget_hide (label [NT_LOC_SWORD]);
                        gtk_widget_show (label [NT_LOC]);
                }
        }

        gtk_dialog_set_response_sensitive (
                GTK_DIALOG (property), GTK_RESPONSE_APPLY, FALSE);
} /* apply_cb */


/*
 * creates one page for the property box.
 */
static GtkWidget *
create_property_table ()
{
        GtkWidget *table;
        GtkWidget *label;
        GtkWidget *combo;
        GtkWidget *widget;
        gint       i;

        table = gtk_table_new (5, 2, FALSE);
        gtk_container_set_border_width (GTK_CONTAINER (table), GNOME_PAD);
        // gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD);
        
        label = gtk_label_new (_("Choose displayed language:"));
        gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

        combo = gtk_combo_box_new_text ();

        i = 0;
        while (languages [i].lang != NULL) {
                if (languages [i].found == TRUE) {
                        gtk_combo_box_append_text (GTK_COMBO_BOX (combo),
                                                   _(languages [i].name));
                        if (strcmp (lang, languages [i].lang) == 0) {
                                gtk_combo_box_set_active
                                        (GTK_COMBO_BOX (combo), i);
                        }
                }
                i++;
        }
        g_signal_connect (G_OBJECT (combo), "changed",
                          G_CALLBACK (lang_changed_cb), NULL);
        gtk_widget_show (combo);
        gtk_table_attach_defaults (GTK_TABLE (table), combo, 1, 2, 0, 1);

        label = gtk_label_new (_("Choose Font:"));
        gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

        widget = gtk_font_button_new ();
        g_signal_connect (G_OBJECT (widget), "font_set",
                          G_CALLBACK (font_sel_cb), NULL);
        if (font != NULL) {
                gtk_font_button_set_font_name (GTK_FONT_BUTTON (widget), font);
        }
        gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, 1, 2);

        widget = gtk_check_button_new_with_label
                (_("Close calendar by double click"));
        if (calendar_close) {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),
                                              TRUE);
        }
        g_signal_connect (G_OBJECT (widget), "toggled",
                          G_CALLBACK (calendar_option_cb), NULL);
        gtk_table_attach_defaults (GTK_TABLE (table), widget, 0, 2, 2, 3);

        widget = gtk_check_button_new_with_label
                (_("Show selective and continuing reading"));
        if (show_readings) {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),
                                              TRUE);
        }
        g_signal_connect (G_OBJECT (widget), "toggled",
                          G_CALLBACK (readings_cb), NULL);
        gtk_table_attach_defaults (GTK_TABLE (table), widget, 0, 2, 3, 4);
        
        widget = gtk_check_button_new_with_label
                (_("Link texts to gnomesword"));
        if (show_sword) {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),
                                              TRUE);
        }
        g_signal_connect (G_OBJECT (widget), "toggled",
                          G_CALLBACK (sword_cb), NULL);
        gtk_table_attach_defaults (GTK_TABLE (table), widget, 0, 2, 4, 5);

        return table;
} /* create_property_table */


/*
 * callback function for options menu.
 */
static void 
lang_changed_cb (GtkWidget *combo, gpointer data)
{
        gint num = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));
        new_lang = (gchar*) languages [num].lang;
        gtk_dialog_set_response_sensitive (
                GTK_DIALOG (property), GTK_RESPONSE_APPLY, TRUE);
} /* lang_changed_cb */


/*
 * callback function when font is selected.
 */
static void 
font_sel_cb (GtkWidget *gfb, gpointer data)
{
        const gchar* font_name = gtk_font_button_get_font_name
                (GTK_FONT_BUTTON (gfb));
        if (font_name != NULL
            && (font == NULL || strcmp (font, font_name) != 0))
        {
                gtk_dialog_set_response_sensitive
                        (GTK_DIALOG (property), GTK_RESPONSE_APPLY, TRUE);
                if (new_font != NULL) {
                        g_free (new_font);
                }
                new_font = g_strdup (font_name);
        }
} /* font_sel_cb */


/*
 * callback function for options menu.
 */
static void 
calendar_option_cb (GtkWidget *toggle, gpointer data)
{
        calendar_close_new = gtk_toggle_button_get_active (
                GTK_TOGGLE_BUTTON (toggle));
        gtk_dialog_set_response_sensitive (
                GTK_DIALOG (property), GTK_RESPONSE_APPLY, TRUE);
} /* calendar_option_cb */


/*
 * callback function for options menu.
 */
static void 
readings_cb (GtkWidget *toggle, gpointer data)
{
        show_readings_new = gtk_toggle_button_get_active (
                GTK_TOGGLE_BUTTON (toggle));
        gtk_dialog_set_response_sensitive (
                GTK_DIALOG (property), GTK_RESPONSE_APPLY, TRUE);
} /* readings_cb */


/*
 * callback function for options menu.
 */
static void 
sword_cb (GtkWidget *toggle, gpointer data)
{
        show_sword_new = gtk_toggle_button_get_active (
                GTK_TOGGLE_BUTTON (toggle));
        gtk_dialog_set_response_sensitive (
                GTK_DIALOG (property), GTK_RESPONSE_APPLY, TRUE);
} /* readings_cb */


/*
 * callback function for exiting the application.
 */
static void 
exit_cb (GtkWidget *w, gpointer data)
{
        gtk_main_quit ();
} /* exit_cb */


/*
 * callback function that displays a calendar.
 */
static void 
calendar_cb (GtkWidget *w, gpointer data)
{
        static GtkWidget *dialog = NULL;
  
        if (dialog != NULL) {
                g_assert (GTK_WIDGET_REALIZED (dialog));
                gdk_window_show  (dialog->window);
                gdk_window_raise (dialog->window);
        } else {
                dialog = gtk_dialog_new ();
                gtk_window_set_title (GTK_WINDOW (dialog), _("Calendar"));
                gtk_dialog_add_action_widget (
                        GTK_DIALOG (dialog),
                        gtk_button_new_from_stock (GTK_STOCK_CLOSE),
                        GTK_RESPONSE_CLOSE);

                calendar = gtk_calendar_new ();
                gtk_calendar_set_display_options
                        (GTK_CALENDAR (calendar), 
                         GTK_CALENDAR_WEEK_START_MONDAY |
                         GTK_CALENDAR_SHOW_WEEK_NUMBERS |
                         GTK_CALENDAR_SHOW_HEADING |
                         GTK_CALENDAR_SHOW_DAY_NAMES);
                gtk_calendar_select_month    (GTK_CALENDAR (calendar), 
                                              g_date_get_month (date) - 1,
                                              g_date_get_year (date));
                gtk_calendar_select_day      (GTK_CALENDAR (calendar),
                                              g_date_get_day (date));
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                                   calendar);
                g_signal_connect (G_OBJECT (dialog), "response",
                                  G_CALLBACK (gtk_widget_destroy), NULL);
                g_signal_connect (G_OBJECT (dialog), "destroy",
                                  G_CALLBACK (gtk_widget_destroyed), &dialog);
                g_signal_connect (G_OBJECT (dialog), "destroy",
                                  G_CALLBACK (gtk_widget_destroyed),
                                  &calendar);
                g_signal_connect (G_OBJECT (calendar),
                                  "day_selected_double_click",
                                  G_CALLBACK (calendar_select_cb), dialog);

                gtk_widget_show_all (dialog);
        }
} /* calendar_cb */


/*
 * callback function that computes Losung for the current date.
 */
static void 
today_cb (GtkWidget *w, gpointer data)
{
        get_time ();
        if (calendar != NULL) {
                gtk_calendar_select_month (GTK_CALENDAR (calendar), 
                                           g_date_get_month (date) - 1,
                                           g_date_get_year (date));
                gtk_calendar_select_day   (GTK_CALENDAR (calendar),
                                           g_date_get_day (date));
        }
        show_text ();
} /* today_cb */


/*
 * callback function that increases the day.
 */
static void 
next_day_cb (GtkWidget *w, gpointer data)
{
        g_date_add_days (date, 1);
        if (calendar != NULL) {
                gtk_calendar_select_month (GTK_CALENDAR (calendar), 
                                           g_date_get_month (date) - 1,
                                           g_date_get_year (date));
                gtk_calendar_select_day   (GTK_CALENDAR (calendar),
                                           g_date_get_day (date));
        }
        show_text ();
} /* next_day_cb */


/*
 * callback function that decreases the day.
 */
static void 
prev_day_cb (GtkWidget *w, gpointer data)
{
        g_date_subtract_days (date, 1);
        if (calendar != NULL) {
                gtk_calendar_select_month (GTK_CALENDAR (calendar), 
                                           g_date_get_month (date) - 1,
                                           g_date_get_year (date));
                gtk_calendar_select_day   (GTK_CALENDAR (calendar),
                                           g_date_get_day (date));
        }
        show_text ();
} /* prev_day_cb */


/*
 * callback function  that increases the month.
 */
static void 
next_month_cb (GtkWidget *w, gpointer data)
{
        g_date_add_months (date, 1);
        if (calendar != NULL) {
                gtk_calendar_select_month (GTK_CALENDAR (calendar), 
                                           g_date_get_month (date) - 1,
                                           g_date_get_year (date));
                gtk_calendar_select_day   (GTK_CALENDAR (calendar),
                                           g_date_get_day (date));
        }
        show_text ();
} /* next_month_cb */


/*
 * callback function that decreases the month.
 */
static void 
prev_month_cb (GtkWidget *w, gpointer data)
{
        g_date_subtract_months (date, 1);
        if (calendar != NULL) {
                gtk_calendar_select_month (GTK_CALENDAR (calendar), 
                                           g_date_get_month (date) - 1,
                                           g_date_get_year (date));
                gtk_calendar_select_day   (GTK_CALENDAR (calendar),
                                           g_date_get_day (date));
        }
        show_text ();
} /* prev_month_cb */


/*
 * callback function for the calendar widget.
 */
static void 
calendar_select_cb (GtkWidget *calendar, gpointer data)
{
        guint y, m, d;

        gtk_calendar_get_date (GTK_CALENDAR (calendar), &y, &m, &d);

        g_date_set_day (date, d);
        g_date_set_month (date, m + 1);
        g_date_set_year (date, y);
        show_text ();

        if (calendar_close) {
                gtk_widget_destroy (GTK_WIDGET (data));
        }
} /* calendar_select_cb */


static void
lang_manager_cb (GtkWidget *w, gpointer data) 
{
        GtkWidget    *dialog;
        GtkWidget    *add_button;
        GtkListStore *store;
        GtkWidget    *list;
        GtkTreeIter   iter1;
        GtkWidget    *hbox;
        GtkWidget    *vbox;
        GtkCellRenderer   *renderer;
        GtkTreeViewColumn *column;
        LosungList* losung_list;
        int i;


        dialog = gtk_dialog_new_with_buttons
                (_("Languages"),
                 GTK_WINDOW (app),
                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                 GTK_STOCK_OK,
                 GTK_RESPONSE_NONE,
                 NULL);

        hbox = gtk_hbox_new (FALSE, 0);
        gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);
        gtk_widget_show (hbox);
        gtk_container_set_border_width (GTK_CONTAINER (hbox), GNOME_PAD);

        store = gtk_list_store_new (1, G_TYPE_STRING);

        losung_list = get_list ();
        for (i = 0; i < losung_list->languages->len; i++) {
                gtk_list_store_append (store, &iter1);
                gtk_list_store_set
                        (store, &iter1, 0,
                         g_ptr_array_index (losung_list->languages, i), -1);
        }

        list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes
                (NULL,
                 renderer,
                 "text", 0,
                 NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
        gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), FALSE);
        gtk_widget_show (list);
        gtk_box_pack_start (GTK_BOX (hbox), list, TRUE, TRUE, GNOME_PAD);



        vbox = gtk_vbutton_box_new ();
        gtk_button_box_set_layout
                (GTK_BUTTON_BOX (vbox),
                 GTK_BUTTONBOX_SPREAD);
        gtk_widget_show (vbox);
        gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, GNOME_PAD);

        add_button = gtk_button_new_from_stock (GTK_STOCK_ADD);
        gtk_widget_show (add_button);
        gtk_container_add (GTK_CONTAINER (vbox), add_button);

        add_button = gtk_button_new_from_stock (GTK_STOCK_REFRESH);
        gtk_widget_show (add_button);
        gtk_container_add (GTK_CONTAINER (vbox), add_button);

        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), hbox);
        
        g_signal_connect (G_OBJECT (dialog), "response",
                          G_CALLBACK (gtk_widget_destroy), NULL);
        gtk_widget_show (dialog);
} /* lang_manager_cb */
