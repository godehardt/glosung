/* glosung.c
 * Copyright (C) 1999-2009 Eicke Godehardt

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

#define GETTEXT_PACKAGE "glosung"
#define PACKAGE "glosung"
#define APPNAME "GLosung"

#define ENABLE_NLS

#include <gtk/gtkversion.h>

#if (GTK_MINOR_VERSION >= (10) && ! defined (WIN32))
  #define VERSE_LINK 1
#endif


#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef WIN32
	#include <gconf/gconf-client.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "parser.h"
#include "download.h"
#include "about.h"
#include "autostart.h"
#include "losunglist.h"


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

#ifndef WIN32
	static GConfClient *client;
#endif /* WIN32 */

static GtkWidget *app;
static GDate     *date;
static GDate     *new_date;
static GtkWidget *calendar;
static GtkWidget *label [NUMBER_OF_LABELS];
static GtkWidget *property = NULL;

static gchar     *new_lang = NULL;
static gchar     *font = NULL;
static gchar     *new_font;
static gboolean   autostart;
static gboolean   autostart_new;
static gboolean   calendar_close = TRUE;
static gboolean   show_readings = TRUE;
static gboolean   show_sword;
static gboolean   show_sword_new;
static gchar     *losung_simple_text;
static guint      losung_simple_text_len;

static LosungList *languages;
static gchar      *lang = NULL;
static GHashTable *lang_translations;
static LosungList *server_list = NULL;


/* static GnomeHelpMenuEntry help_ref = { "glosung", "pbox.html" }; */

/****************************\
      Function prototypes
\****************************/

static GHashTable *init_languages            (void);

static void       get_time              (void);
static void       show_text             (void);
static void       create_app            (void);

static void add_lang_cb          (GtkWidget *w,   gpointer data);
static void about_cb             (GtkWidget *w,   gpointer data);
static void about_herrnhut_cb    (GtkWidget *w,   gpointer data);
static void calendar_cb          (GtkWidget *w,   gpointer data);
static void calendar_select_cb   (GtkWidget *calendar, gpointer data);
static GString* create_years_string (gchar *langu);
static void update_years         (GtkWidget *w,   gpointer data);
static void lang_manager_cb      (GtkWidget *w,   gpointer data);
static void link_execute         (GtkWidget *widget,
                                  gchar     *uri, gpointer data);
static void next_day_cb          (GtkWidget *w,   gpointer data);
static void next_month_cb        (GtkWidget *w,   gpointer data);
static void no_languages_cb      (GtkWidget *w,   gpointer data);
static void prev_day_cb          (GtkWidget *w,   gpointer data);
static void prev_month_cb        (GtkWidget *w,   gpointer data);
static void property_cb          (GtkWidget *w,   gpointer data);
static void today_cb             (GtkWidget *w,   gpointer data);
static void update_language_store ();
static void clipboard_cb         (GtkWidget *w,   gpointer data);
static void my_wrap              (gchar     *string);

void autostart_cb         (GtkWidget *w,   gpointer data);
void font_sel_cb          (GtkWidget *button,   gpointer data);
void lang_changed_cb      (GtkWidget *item,     gpointer data);
void property_response_cb (GtkDialog *dialog,
                           gint       arg1,
                           gpointer   user_data);
void sword_cb             (GtkWidget *toggle,   gpointer data);


/******************************\
       Set up the GUI
\******************************/

#define MY_PAD 10
#define WRAP_SIZE 47


static gchar* uistring =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='FileMenu'>"
        "      <menuitem action='Calendar'/>"
        "      <separator/>"
        "      <menuitem action='Today'/>"
        "      <menuitem action='NextDay'/>"
        "      <menuitem action='PrevDay'/>"
        "      <menuitem action='NextMonth'/>"
        "      <menuitem action='PrevMonth'/>"
        "      <separator/>"
        "      <menuitem action='Quit'/>"
        "    </menu>"
        "    <menu action='EditMenu'>"
        "      <menuitem action='Copy'/>"
        "      <separator/>"
        "      <menuitem action='Copy'/>"
        "      <menuitem action='Organize'/>"
        "      <menuitem action='Preferences'/>"
        "    </menu>"
        "    <menu action='HelpMenu'>"
        "      <menuitem action='AboutHerrnhut'/>"
        "      <separator/>"
        "      <menuitem action='About'/>"
        "    </menu>"
        "  </menubar>"
        "  <toolbar name='ToolBar'>"
        "    <toolitem action='Quit'/>"
        "    <toolitem action='PrevMonth'/>"
        "    <toolitem action='PrevDay'/>"
        "    <toolitem action='Today'/>"
        "    <toolitem action='NextDay'/>"
        "    <toolitem action='NextMonth'/>"
        "  </toolbar>"
        "</ui>";

static GtkActionEntry entries[] = {
        { "FileMenu", NULL, N_("_File") },
        { "EditMenu", NULL, N_("_Edit") },
        { "HelpMenu", NULL, N_("_Help") },

        { "Calendar", GTK_STOCK_COPY, N_("_Calendar"), "<control>D",
          N_("select date from calendar"), G_CALLBACK (calendar_cb) },
        { "Today", GTK_STOCK_REFRESH, N_("_Today"), "Home",
          N_("Show losung of today"), G_CALLBACK (today_cb) },
        { "NextDay", GTK_STOCK_GO_FORWARD, N_("_Next Day"), "Page_Down",
          N_("Show losung of next day"), G_CALLBACK (next_day_cb) },
        { "PrevDay", GTK_STOCK_GO_BACK, N_("_Previous Day"), "Page_Up",
          N_("Show losung of previous day"), G_CALLBACK (prev_day_cb) },
        { "NextMonth", GTK_STOCK_GOTO_LAST, N_("_Next Month"),
          "<control>Page_Down",
          N_("Show losung of next month"), G_CALLBACK (next_month_cb) },
        { "PrevMonth", GTK_STOCK_GOTO_FIRST, N_("_Previous Month"),
          "<control>Page_Up",
          N_("Show losung of previous month"), G_CALLBACK (prev_month_cb) },
        { "Quit", GTK_STOCK_QUIT,  NULL, "<control>Q",
          N_("Quit the application"), G_CALLBACK (gtk_main_quit) },

        { "Copy", GTK_STOCK_COPY, N_("_Copy text"), "<control>C",
          N_("copy the text to clipboard"), G_CALLBACK (clipboard_cb) },
        { "Organize", NULL, N_("_Organize Watchwords..."), NULL,
          N_("update/install text files"), G_CALLBACK (lang_manager_cb) },
        { "Preferences", GTK_STOCK_PREFERENCES, NULL, NULL,
          N_("Edit the preferences"), G_CALLBACK (property_cb) },

        { "AboutHerrnhut", GTK_STOCK_ABOUT, N_("About Herrnhut"), NULL,
          N_("about Losungen"), G_CALLBACK (about_herrnhut_cb) },
        { "About", GTK_STOCK_ABOUT, N_("About GLosung"), NULL,
          N_("about GLosung"), G_CALLBACK (about_cb) },
};


static guint n_entries = G_N_ELEMENTS (entries);



/*
 * the one and only main function :-)
 */
int
main (int argc, char **argv)
{
        GError *error = NULL;

        /* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, "/usr/share/locale");
        bind_textdomain_codeset (PACKAGE, "UTF-8");
        textdomain (PACKAGE);

        /*
        {
        gchar **list = g_get_language_names ();
        int i;

        for (i = 0; list [i]; i++) {
                g_message (list [i]);
        }
        }
        */

        gboolean once = FALSE;
        GOptionEntry options[] = {
                { "once", '1', 0, G_OPTION_ARG_NONE, &once,
                  N_("Start only once a day"), NULL },
                { NULL }
        };
        gtk_init_with_args (&argc, &argv, "[--once]", options, NULL, &error);
        if (error) {
                fprintf (stderr, "Error: %s\n", error->message);
                g_error_free (error);
                exit (1);
        }

#ifndef WIN32
        client = gconf_client_get_default ();
#endif /* WIN32 */

        date = g_date_new ();
        get_time ();
        new_date = g_date_new_julian (g_date_get_julian (date));
#ifndef WIN32
        if (once) {
                gchar *last_time_str = gconf_client_get_string
                        (client, "/apps/" PACKAGE "/last_time", NULL);
                if (last_time_str) {
                        GDate *last_time = g_date_new ();
                        g_date_set_parse (last_time, last_time_str);
                        if (g_date_compare (last_time, date) >= 0) {
                                exit (0);
                        }
                }
        }
        gchar *time_str = g_malloc (11); /* "YYYY-MM-DD"; */
        g_date_strftime (time_str, 11, "%Y-%m-%d", date);
        gconf_client_set_string
                (client, "/apps/" PACKAGE "/last_time", time_str, NULL);
#endif /* WIN32 */

        gtk_window_set_default_icon_from_file
                (PACKAGE_PIXMAPS_DIR "glosung.png", NULL);

        lang_translations = init_languages ();
        languages         = scan_for_files ();

#ifndef WIN32
        lang = gconf_client_get_string
                (client, "/apps/" PACKAGE "/language", NULL);
#endif /* WIN32 */
        if (lang == NULL) {
                /* should be translated to corresponding language, e.g. 'de' */
                lang = _("en");

                /* is requested language available,
                   if not use first available language instead */

                if (languages->languages->len > 0 &&
                    ! g_hash_table_lookup (languages->hash_table, lang)) {
                        lang = g_ptr_array_index (languages->languages, 0);
                }
        }

        printf ("Choosen language: %s\n", lang);
#ifndef WIN32
        calendar_close = gconf_client_get_bool
                (client, "/apps/" PACKAGE "/calendar_close_by_double_click",
                 NULL);
        font = gconf_client_get_string (client, "/apps/" PACKAGE "/font",
                                        NULL);
#endif /* WIN32 */
#if (defined (VERSE_LINK) && ! defined (WIN32))
        show_sword = show_sword_new = gconf_client_get_bool
                (client, "/apps/" PACKAGE "/link_sword", NULL);
#else
        show_sword = show_sword_new = FALSE;
#endif /* VERSE_LINK, WIN32 */

        create_app ();
        if (languages->languages->len == 0) {
                GtkWidget *error = gtk_message_dialog_new
                        (GTK_WINDOW (app), GTK_DIALOG_DESTROY_WITH_PARENT,
                         GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                         _("No text files found!\n"
                           "Please install your prefered languages."));
                g_signal_connect (G_OBJECT (error), "response",
                                  G_CALLBACK (no_languages_cb), error);
                               // G_CALLBACK (gtk_widget_destroy), NULL);
                               // G_CALLBACK (lang_manager_cb), NULL);
                               // CALLBACK (gtk_main_quit), NULL);
                gtk_widget_show (error);
        } else {
                show_text ();
        }

        gtk_main ();

        /* exit (EXIT_SUCCESS); */
        return 0;
} /* main */


static GHashTable *
init_languages (void)
{
        GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);
        g_hash_table_insert (ht, "af",    _("afrikaans"));
        g_hash_table_insert (ht, "ar",    _("arabic"));
        g_hash_table_insert (ht, "cs",    _("czech"));
        g_hash_table_insert (ht, "de",    _("german"));
        g_hash_table_insert (ht, "en",    _("english"));
        g_hash_table_insert (ht, "es",    _("spanish"));
        g_hash_table_insert (ht, "fr",    _("french"));
        g_hash_table_insert (ht, "he",    _("hebrew"));
        g_hash_table_insert (ht, "hu",    _("hungarian"));
        g_hash_table_insert (ht, "it",    _("italian"));
        g_hash_table_insert (ht, "nl",    _("dutch"));
        g_hash_table_insert (ht, "no",    _("norwegian"));
        g_hash_table_insert (ht, "pt",    _("portuguese"));
        g_hash_table_insert (ht, "ro",    _("romanian"));
        g_hash_table_insert (ht, "ru",    _("russian"));
        g_hash_table_insert (ht, "ta",    _("tamil"));
        g_hash_table_insert (ht, "tr",    _("turkish"));
        g_hash_table_insert (ht, "vi",    _("vietnamese"));
        g_hash_table_insert (ht, "zh-CN", _("chinese simplified"));
        g_hash_table_insert (ht, "zh-TW", _("chinese traditional"));

        return ht;
} /* init_languages */


/*
 * does all the GUI stuff.
 */
static void
create_app (void)
{
        GtkActionGroup *action;
        GtkUIManager   *uiman;
        GtkWidget      *vbox;
        GtkWidget      *content;
        GtkWidget      *menubar;
        GtkWidget      *toolbar;
        gint            i;
        PangoFontDescription *font_desc;

        app = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        g_signal_connect (G_OBJECT (app), "destroy",
                          G_CALLBACK (gtk_main_quit),
                          NULL);
        gtk_window_set_title (GTK_WINDOW (app), APPNAME);
        gtk_window_set_default_size (GTK_WINDOW (app), 500, 400);
        gtk_window_set_position (GTK_WINDOW (app), GTK_WIN_POS_CENTER);

        vbox = gtk_vbox_new (FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
        gtk_container_add (GTK_CONTAINER (app), vbox);

        action = gtk_action_group_new ("GLosungActions");
        gtk_action_group_set_translation_domain (action, PACKAGE);
        gtk_action_group_add_actions (action, entries, n_entries, NULL);

        uiman = gtk_ui_manager_new ();
        gtk_ui_manager_insert_action_group (uiman, action, 0);
        gtk_window_add_accel_group (GTK_WINDOW (app),
                                    gtk_ui_manager_get_accel_group (uiman));

        GError *error;
        if (! gtk_ui_manager_add_ui_from_string (uiman, uistring, -1, &error))
        {
                g_message ("building menus failed: %s", error->message);
                g_error_free (error);
        }
        menubar = gtk_ui_manager_get_widget (uiman, "/MenuBar");
        gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, TRUE, 0);

        toolbar = gtk_ui_manager_get_widget (uiman, "/ToolBar");
        gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);

        content = gtk_vbox_new (FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (content), MY_PAD);
        gtk_box_pack_start (GTK_BOX (vbox), content, FALSE, FALSE, 0);

        for (i = 0; i < NUMBER_OF_LABELS; i++) {
                if (i == OT_LOC_SWORD || i == NT_LOC_SWORD) {
#ifdef VERSE_LINK
                        GtkWidget *widget;
                        widget = gtk_hbutton_box_new ();
                        gtk_button_box_set_layout
                                (GTK_BUTTON_BOX (widget),
                                 GTK_BUTTONBOX_SPREAD);
                        label [i] = gtk_link_button_new ("");
                        gtk_link_button_set_uri_hook
                                ((GtkLinkButtonUriFunc) link_execute,
                                 NULL, NULL);
                        gtk_container_add (GTK_CONTAINER (widget), label [i]);
                        gtk_box_pack_start (GTK_BOX (content), widget,
                                            FALSE, FALSE, 0);
#endif
                } else {
                        label [i] = gtk_label_new ("");
                        gtk_label_set_justify (GTK_LABEL (label [i]),
                                               GTK_JUSTIFY_CENTER);
                        gtk_box_pack_start (GTK_BOX (content), label [i],
                                            FALSE, FALSE, 0);
                }
        }
        if (font != NULL) {
                font_desc = pango_font_description_from_string (font);
                for (i = 0; i < NUMBER_OF_LABELS; i++) {
                        if (i != OT_LOC_SWORD && i != NT_LOC_SWORD) {
                                gtk_widget_modify_font (label [i], font_desc);
                        }
                }
                pango_font_description_free (font_desc);
        }

        gtk_widget_show_all (app);
        if (show_sword) {
                gtk_widget_hide (label [OT_LOC]);
                gtk_widget_hide (label [NT_LOC]);
        } else {
#ifdef VERSE_LINK
                gtk_widget_hide (label [OT_LOC_SWORD]);
                gtk_widget_hide (label [NT_LOC_SWORD]);
#endif
        }
        if (! show_readings) {
                gtk_widget_hide (label [X3]);
                gtk_widget_hide (label [READING]);
        }
} /* create_app */


/*
 * computes local date and fill GDate date.
 */
static void
get_time (void)
{
        time_t     t;
        struct tm  now;

        t = time (NULL);
        now = *localtime (&t);
        g_date_set_dmy (date,
                        now.tm_mday, now.tm_mon + 1, now.tm_year + 1900);
} /* get_time */


/*
 * display the ww for the current date.
 */
static void
show_text (void)
{
        const Losung *ww;

        ww = get_orig_losung (new_date, lang);
        if (! ww) {
                ww = get_losung (new_date, lang);
                if (! ww) {
                        ww = get_the_word (new_date, lang);
                }
        } else {
                my_wrap (ww->ot.text);
                my_wrap (ww->nt.text);
        }

        if (ww == NULL) {
                GtkWidget *error = gtk_message_dialog_new (
                        GTK_WINDOW (app), GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                        _("No %s texts found for %d!"),
                        (gchar*)g_hash_table_lookup (lang_translations, lang),
                        g_date_get_year (new_date));
                g_signal_connect (G_OBJECT (error), "response",
                                  G_CALLBACK (gtk_widget_destroy), NULL);
                gtk_widget_show (error);
                new_date = g_date_new_julian (g_date_get_julian (date));
                return;
        }

        date = g_date_new_julian (g_date_get_julian (new_date));

        gtk_label_set_text (GTK_LABEL (label [TITLE]), ww->title);
        if (ww->ot.say != NULL) {
                gchar *text;

                text = g_strconcat (ww->ot.say, "\n", ww->ot.text, NULL);
                gtk_label_set_text (GTK_LABEL (label [OT_TEXT]), text);
                g_free (text);
        } else {
                gtk_label_set_text (GTK_LABEL (label [OT_TEXT]), ww->ot.text);
        }

#ifdef VERSE_LINK
        gtk_button_set_label (GTK_BUTTON (label [OT_LOC_SWORD]),
                             ww->ot.location);
        if (ww->ot.location_sword) {
                gtk_link_button_set_uri
                        (GTK_LINK_BUTTON (label [OT_LOC_SWORD]),
                         ww->ot.location_sword);
                if (show_sword) {
                        gtk_widget_hide (label [OT_LOC]);
                        gtk_widget_show (label [OT_LOC_SWORD]);
                }
        } else {
                gtk_widget_hide (label [OT_LOC_SWORD]);
                gtk_widget_show (label [OT_LOC]);
        }
#endif
        gtk_label_set_text (GTK_LABEL (label [OT_LOC]), ww->ot.location);

        if (ww->nt.say != NULL) {
                gchar *text;

                text = g_strconcat (ww->nt.say, "\n", ww->nt.text, NULL);
                gtk_label_set_text (GTK_LABEL (label [NT_TEXT]), text);
                g_free (text);
        } else {
                gtk_label_set_text (GTK_LABEL (label [NT_TEXT]), ww->nt.text);
        }
#ifdef VERSE_LINK
        gtk_button_set_label (GTK_BUTTON (label [NT_LOC_SWORD]),
                             ww->nt.location);
        if (ww->nt.location_sword) {
                gtk_link_button_set_uri
                        (GTK_LINK_BUTTON (label [NT_LOC_SWORD]),
                         ww->nt.location_sword);
                if (show_sword) {
                        gtk_widget_hide (label [NT_LOC]);
                        gtk_widget_show (label [NT_LOC_SWORD]);
                }
        } else {
                gtk_widget_hide (label [NT_LOC_SWORD]);
                gtk_widget_show (label [NT_LOC]);
        }
#endif
        gtk_label_set_text (GTK_LABEL (label [NT_LOC]), ww->nt.location);

        if (ww->comment) {
                GtkWidget *comment;

                comment = gtk_message_dialog_new (
                        GTK_WINDOW (app), GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                        "%s", ww->comment);

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
        } else {
                gtk_label_set_text  (GTK_LABEL (label [READING]), "");
                gtk_widget_hide (label [X3]);
                gtk_widget_hide (label [READING]);
        }

        gint i;
        for (i = 0; i < NUMBER_OF_LABELS; i++) {
                if (i != OT_LOC_SWORD && i != NT_LOC_SWORD) {
                        gtk_label_set_use_markup (GTK_LABEL (label [i]), TRUE);
                }
        }

        GString* string = g_string_new ("");
        g_string_append (string, ww->title);
        g_string_append (string, "\n\n");
        if (ww->ot.say != NULL) {
                g_string_append (string, ww->ot.say);
                g_string_append (string, "\n");
        }
        g_string_append (string, ww->ot.text);
        g_string_append (string, "\n");
        g_string_append (string, ww->ot.location);
        g_string_append (string, "\n\n");

        if (ww->nt.say != NULL) {
                g_string_append (string, ww->nt.say);
                g_string_append (string, "\n");
        }
        g_string_append (string, ww->nt.text);
        g_string_append (string, "\n");
        g_string_append (string, ww->nt.location);
        g_string_append (string, "\n");

        losung_simple_text_len = string->len;
        losung_simple_text = g_string_free (string, FALSE);

        losung_free (ww);
} /* show_text */


/*
 * callback function for linkbutton to open gnomesword
 */
static void
link_execute (GtkWidget *widget, gchar *uri, gpointer data)
{
        /* since 2.14 !!!
        gboolean gtk_show_uri (NULL, (const gchar*) uri, 0, NULL);
         */
        /* hack because of gtk_about_dialog_set_url_hook */
        if (strncmp (uri, "http://", 7) == 0) {
                     return;
        }
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
                            (GTK_WINDOW (app), GTK_DIALOG_DESTROY_WITH_PARENT,
                             GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                 "%s", error->message);
                        //_("No text files found!\n");
                        g_signal_connect (G_OBJECT (msg), "response",
                                          G_CALLBACK (gtk_widget_destroy),NULL);
                        gtk_widget_show (msg);
                        g_error_free (error);
                }
        }
}


/*
 * callback function that displays the about dialog.
 */
static void
about_cb (GtkWidget *w, gpointer data)
{
        about (app);
} /* about_cb */


/*
 * callback function that displays the about dialog.
 */
static void
about_herrnhut_cb (GtkWidget *w, gpointer data)
{
        about_herrnhut (app);
} /* about_herrnhut_cb */


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
                GtkWidget *combo;
                gint       i;

                if (new_font != NULL) {
                        g_free (new_font);
                        new_font = NULL;
                }

                GtkBuilder* builder = gtk_builder_new ();
                gtk_builder_set_translation_domain (builder, PACKAGE);
                guint build = gtk_builder_add_from_file
                        (builder, "ui/preferences.glade", NULL);
                if (! build) {
                        g_message ("Error while loading UI definition file");
                        return;
                }
                property = GTK_WIDGET
                        (gtk_builder_get_object (builder, "preferences_dialog"));

                combo = GTK_WIDGET
                        (gtk_builder_get_object (builder, "language_combobox"));

                GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
                gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo),
                renderer, TRUE);
                gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo),
                renderer, "text", 0);

                for (i = 0; i < (languages->languages)->len; i++) {
                        gchar *langu = g_ptr_array_index (languages->languages, i);
                        gtk_combo_box_append_text
                                (GTK_COMBO_BOX (combo),
                                 g_hash_table_lookup (lang_translations, langu));
                        if (strcmp (lang, langu) == 0) {
                                gtk_combo_box_set_active
                                        (GTK_COMBO_BOX (combo), i);
                        }
                }

                if (font != NULL) {
                        gtk_font_button_set_font_name (GTK_FONT_BUTTON
                           (gtk_builder_get_object (builder, "fontbutton")),
                            font);
                }
                if (is_in_autostart ()) {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                           (gtk_builder_get_object (builder, "autostart_checkbox")),
                            TRUE);
                }
#ifdef VERSE_LINK
                if (show_sword) {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                           (gtk_builder_get_object (builder, "sword_checkbox")),
                            TRUE);
                }
#else
                gtk_widget_set_sensitive (GTK_WIDGET
                        (gtk_builder_get_object (builder, "sword_checkbox")),
                         FALSE);
#endif

/*
                g_signal_connect (G_OBJECT (property), "destroy",
                                 G_CALLBACK (gtk_widget_destroyed), &property);
*/

                gtk_builder_connect_signals (builder, NULL);
                gtk_widget_show_all (property);
        }
} /* property_cb */


/*
 * callback function for property box.
 */
void
property_response_cb (GtkDialog *dialog, gint arg1, gpointer user_data)
{
        switch (arg1) {
        case GTK_RESPONSE_HELP:
                break;
        case GTK_RESPONSE_CLOSE:
                gtk_widget_destroy (GTK_WIDGET (dialog));
                break;
        }
} /* property_response_cb */


/*
 * callback function for options menu.
 */
void
lang_changed_cb (GtkWidget *combo, gpointer data)
{
        gint num = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));
        new_lang = (gchar*) g_ptr_array_index (languages->languages, num);
        if (new_lang != NULL) {
                lang = new_lang;
                new_lang = NULL;
#ifndef WIN32
                gconf_client_set_string
                        (client, "/apps/" PACKAGE "/language", lang, NULL);
#endif /* WIN32 */
                show_text ();
        }
} /* lang_changed_cb */


/*
 * callback function when font is selected.
 */
void
font_sel_cb (GtkWidget *gfb, gpointer data)
{
        const gchar* font_name = gtk_font_button_get_font_name
                (GTK_FONT_BUTTON (gfb));
        if (font_name != NULL
            && (font == NULL || strcmp (font, font_name) != 0))
        {
                if (new_font != NULL) {
                        g_free (new_font);
                }
                new_font = g_strdup (font_name);
                if (new_font != NULL) {
                        PangoFontDescription *font_desc;
                        gint i;

                        if (font != NULL) {
                                g_free (font);
                        }
                        font = new_font;
                        new_font = NULL;
#ifndef WIN32
                        gconf_client_set_string
                                (client, "/apps/" PACKAGE "/font", font, NULL);
#endif /* WIN32 */

                        font_desc = pango_font_description_from_string (font);
                        for (i = 0; i < NUMBER_OF_LABELS; i++) {
                                gtk_widget_modify_font (label [i], font_desc);
                        }
                        pango_font_description_free (font_desc);
                }
        }
} /* font_sel_cb */


/*
 * callback function for autostart changes.
 */
void
autostart_cb (GtkWidget *toggle, gpointer data)
{
        autostart_new = gtk_toggle_button_get_active (
                GTK_TOGGLE_BUTTON (toggle));
        if (autostart_new != autostart) {
                if (autostart_new) {
                        /* TODO handle return type */
                        add_to_autostart ();
                } else {
                        /* TODO handle return type */
                        remove_from_autostart ();
                }
                autostart = autostart_new;
        }
} /* autostart_cb */


/*
 * callback function for gnomwsword option change.
 */
void
sword_cb (GtkWidget *toggle, gpointer data)
{
        show_sword_new = gtk_toggle_button_get_active
                (GTK_TOGGLE_BUTTON (toggle));

#ifdef VERSE_LINK
        if (show_sword_new != show_sword) {
                show_sword = show_sword_new;
#ifndef WIN32
                gconf_client_set_bool (client, "/apps/" PACKAGE "/link_sword",
                                       show_sword, NULL);
#endif /* WIN32 */
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
#endif
} /* sword_cb */


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
        date = g_date_new ();
        get_time ();
        new_date = g_date_new_julian (g_date_get_julian (date));
        show_text ();
        if (calendar != NULL) {
                gtk_calendar_select_month (GTK_CALENDAR (calendar),
                                           g_date_get_month (date) - 1,
                                           g_date_get_year (date));
                gtk_calendar_select_day   (GTK_CALENDAR (calendar),
                                           g_date_get_day (date));
        }
} /* today_cb */


/*
 * callback function that increases the day.
 */
static void
next_day_cb (GtkWidget *w, gpointer data)
{
        g_date_add_days (new_date, 1);
        show_text ();
        if (calendar != NULL) {
                gtk_calendar_select_month (GTK_CALENDAR (calendar),
                                           g_date_get_month (date) - 1,
                                           g_date_get_year (date));
                gtk_calendar_select_day   (GTK_CALENDAR (calendar),
                                           g_date_get_day (date));
        }
} /* next_day_cb */


/*
 * callback function that decreases the day.
 */
static void
prev_day_cb (GtkWidget *w, gpointer data)
{
        g_date_subtract_days (new_date, 1);
        show_text ();
        if (calendar != NULL) {
                gtk_calendar_select_month (GTK_CALENDAR (calendar),
                                           g_date_get_month (date) - 1,
                                           g_date_get_year (date));
                gtk_calendar_select_day   (GTK_CALENDAR (calendar),
                                           g_date_get_day (date));
        }
} /* prev_day_cb */


/*
 * callback function  that increases the month.
 */
static void
next_month_cb (GtkWidget *w, gpointer data)
{
        g_date_add_months (new_date, 1);
        show_text ();
        if (calendar != NULL) {
                gtk_calendar_select_month (GTK_CALENDAR (calendar),
                                           g_date_get_month (date) - 1,
                                           g_date_get_year (date));
                gtk_calendar_select_day   (GTK_CALENDAR (calendar),
                                           g_date_get_day (date));
        }
} /* next_month_cb */


/*
 * callback function that decreases the month.
 */
static void
prev_month_cb (GtkWidget *w, gpointer data)
{
        g_date_subtract_months (new_date, 1);
        show_text ();
        if (calendar != NULL) {
                gtk_calendar_select_month (GTK_CALENDAR (calendar),
                                           g_date_get_month (date) - 1,
                                           g_date_get_year (date));
                gtk_calendar_select_day   (GTK_CALENDAR (calendar),
                                           g_date_get_day (date));
        }
} /* prev_month_cb */


/*
 * callback function for the calendar widget.
 */
static void
calendar_select_cb (GtkWidget *calendar, gpointer data)
{
        guint y, m, d;

        gtk_calendar_get_date (GTK_CALENDAR (calendar), &y, &m, &d);

        g_date_set_day (new_date, d);
        g_date_set_month (new_date, m + 1);
        g_date_set_year (new_date, y);
        show_text ();

        if (calendar_close) {
                gtk_widget_destroy (GTK_WIDGET (data));
        }
} /* calendar_select_cb */







static GtkWidget    *lang_combo;
static GtkComboBox  *year_combo;
static GtkListStore *store;
static GPtrArray    *years;


static void
lang_manager_cb (GtkWidget *w, gpointer data)
{
        GtkWidget    *dialog;
        GtkWidget    *scroll;
        GtkWidget    *add_button;
        GtkWidget    *list;
        GtkWidget    *vbox;
        GtkCellRenderer   *renderer;
        GtkTreeViewColumn *column;

        dialog = gtk_dialog_new_with_buttons
                (_("Languages"),
                 GTK_WINDOW (app),
                 // GTK_DIALOG_MODAL |
                 GTK_DIALOG_DESTROY_WITH_PARENT,
                 GTK_STOCK_OK,
                 GTK_RESPONSE_NONE,
                 NULL);

        vbox = gtk_vbox_new (FALSE, 0);
        // gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
        gtk_widget_show (vbox);
        gtk_container_set_border_width (GTK_CONTAINER (vbox), MY_PAD);

        store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
        update_language_store (store);

        list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes
                (NULL, renderer, "text", 0, NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes
                (NULL, renderer, "text", 1, NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
        gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), FALSE);
        gtk_widget_set_size_request (list, 200, 140);
        gtk_widget_show (list);

        GtkWidget *lang_frame = gtk_frame_new (_("Installed Languages"));
        gtk_widget_show (lang_frame);

        scroll = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_show (scroll);

        gtk_container_add (GTK_CONTAINER (lang_frame), scroll);
        gtk_container_add (GTK_CONTAINER (scroll), list);

        gtk_box_pack_start (GTK_BOX (vbox), lang_frame, TRUE, TRUE, MY_PAD);

        add_button = gtk_button_new_from_stock (GTK_STOCK_ADD);
        g_signal_connect (G_OBJECT (add_button), "clicked",
                          G_CALLBACK (add_lang_cb), dialog);
        gtk_widget_show (add_button);
        gtk_box_pack_start (GTK_BOX (vbox), add_button,
                            FALSE, FALSE, MY_PAD);

        gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), vbox);

        g_signal_connect (G_OBJECT (dialog), "response",
                          G_CALLBACK (gtk_widget_destroy), NULL);
        gtk_widget_show (dialog);
} /* lang_manager_cb */


static void
add_lang_cb (GtkWidget *w, gpointer data)
{
        GtkWidget    *dialog;

        GtkBuilder* builder = gtk_builder_new ();
        guint build = gtk_builder_add_from_file
                (builder, "ui/add_language.glade", NULL);
        if (! build) {
                g_message ("Error while loading UI definition file");
                return;
        }
        dialog = GTK_WIDGET
                (gtk_builder_get_object (builder, "add_language_dialog"));

	/*
        lang_combo = gtk_combo_box_new_text ();

	gchar *langu = g_ptr_array_index (server_list->languages, i);
	// gchar *str = g_strdup_printf ("%s ()", );
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (lang_combo), "German Watchwords");
	if (lang != NULL && strcmp (lang, langu) == 0) {
		gtk_combo_box_set_active (GTK_COMBO_BOX (lang_combo), i);
	}
	*/

        GtkFrame *year_frame = GTK_FRAME
                (gtk_builder_get_object (builder, "year_frame"));
	year_combo = GTK_COMBO_BOX (gtk_combo_box_new_text ());
        gtk_container_add (GTK_CONTAINER (year_frame), GTK_WIDGET (year_combo));
	gint i;
        gint years [] = {2009, 2008, 2007};
	gint this_year = 2009;
        for (i = this_year; i >= this_year - 2; i--) {
                gchar *year = g_strdup_printf ("%d", i);
                gtk_combo_box_append_text (year_combo, year);
        }
        gtk_combo_box_set_active (GTK_COMBO_BOX (year_combo), 0);

        /*
        g_signal_connect (G_OBJECT (lang_combo), "changed",
                          G_CALLBACK (update_years), year_combo);
        */

        gtk_widget_show_all (dialog);

        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
                gchar *langu = "de";
/*                 gchar *langu = g_ptr_array_index
 *                       (server_list->languages,
 *                        gtk_combo_box_get_active (GTK_COMBO_BOX (lang_combo)));
 *                 gint year = GPOINTER_TO_INT (g_ptr_array_index (years,
 *                        gtk_combo_box_get_active (GTK_COMBO_BOX (year_combo))));
 */
                gint year = years
                        [gtk_combo_box_get_active (GTK_COMBO_BOX (year_combo))];
                download_losungen (year);
                losunglist_add (languages, langu, year);
                losunglist_finialize (languages);
                update_language_store ();
                if (languages->languages->len == 1) {
                        lang = langu;
#ifndef WIN32
                        gconf_client_set_string
                            (client, "/apps/" PACKAGE "/language", lang, NULL);
#endif /* WIN32 */
                }
                show_text ();
        }
        gtk_widget_destroy (dialog);
} /* add_lang_cb */


static void
no_languages_cb (GtkWidget *w, gpointer data)
{
        gtk_widget_destroy (w);
        lang_manager_cb (app, data);
} /* no_languages_cb */


static void
update_years (GtkWidget *w, gpointer data)
{
        GtkComboBox    *combo = GTK_COMBO_BOX (year_combo);
        while (gtk_combo_box_get_active (combo) != -1) {
                gtk_combo_box_remove_text (combo, 0);
                gtk_combo_box_set_active (combo, 0);
        }

        gchar *langu = g_ptr_array_index
                (server_list->languages,
                 gtk_combo_box_get_active (GTK_COMBO_BOX (lang_combo)));
        years = g_hash_table_lookup (server_list->hash_table,langu);
        gint i;
        for (i = 0; i < years->len; i++) {
                gchar *year = g_strdup_printf
                        ("%d", GPOINTER_TO_INT (g_ptr_array_index (years, i)));
                gtk_combo_box_append_text (combo, year);
        }
        gtk_combo_box_set_active (GTK_COMBO_BOX (year_combo), 0);
} /* update_years */


static GString*
create_years_string (gchar *langu)
{
        GPtrArray *years = g_hash_table_lookup (languages->hash_table, langu);
        GString *result = g_string_new ("");
        g_string_printf (result, "%d",
                         GPOINTER_TO_INT (g_ptr_array_index (years, 0)));

        gint i;
        for (i = 1; i < years->len; i++) {
                g_string_append_printf
                        (result, ", %d",
                         GPOINTER_TO_INT (g_ptr_array_index (years, i)));
        }

        return result;
} /* create_years_string */


static void
update_language_store ()
{
        GtkTreeIter   iter1;
        int i;

        gtk_list_store_clear (store);
        for (i = 0; i < languages->languages->len; i++) {
                gtk_list_store_append (store, &iter1);
                gchar *langu = g_ptr_array_index (languages->languages, i);
                GString *years = create_years_string (langu);
                gtk_list_store_set
                        (store, &iter1,
                         0, g_hash_table_lookup (lang_translations, langu),
                         1, years->str,
                         -1);
                g_string_free (years, TRUE);
        }
} /* update_language_store */


static void
clipboard_cb (GtkWidget *w, gpointer data)
{
        GtkClipboard* clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
        gtk_clipboard_set_text (clipboard,
                                losung_simple_text, losung_simple_text_len);
        clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
        gtk_clipboard_set_text (clipboard,
                                losung_simple_text, losung_simple_text_len);
} /* clipboard_cb */


static void
my_wrap (gchar *string)
{
        int len = strlen (string);
        int index = 0;

        while (index + WRAP_SIZE < len) {
                index += WRAP_SIZE;
                while (string [index] != ' ') {
                        index--;
                }
                string [index] = '\n';
        }
} /* my_wrap */
