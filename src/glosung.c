/* glosung.c
 * Copyright (C) 1999-2022 Eicke Godehardt

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

/* FIXME */
#define GETTEXT_PACKAGE "glosung"
#define PACKAGE "glosung"
#define APPNAME "GLosung"

#define ENABLE_NLS


#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <adwaita.h>

#include "parser.h"
#include "download.h"
#include "about.h"
#include "autostart.h"
#include "collections.h"
#include "settings.h"
#include "util.h"

#if (! defined (WIN32) && ! defined (__APPLE__))
    #define VERSE_LINK 1
#else
    #undef VERSE_LINK
#endif


/****************************\
   Variables & Definitions
\****************************/

enum {
        TITLE,
        OT_TEXT,
        OT_LOC,
        OT_LOC_SWORD,
        NT_TEXT,
        NT_LOC,
        NT_LOC_SWORD,
        READING,
        NUMBER_OF_LABELS
};

static const int UPDATE_TIME = 60;  /* in secs */
static const int WRAP_LENGTH = 47;

static gboolean   once = FALSE;

static GtkWidget *app;
static GDateTime *date;
static GtkWidget *calendar;
static GtkWidget *label [NUMBER_OF_LABELS];

static gchar     *new_lang = NULL;
static gchar     *font = NULL;
static gchar     *new_font;
static gboolean   calendar_close = TRUE;
static gboolean   show_readings = TRUE;
static gboolean   show_sword;
static gboolean   show_sword_new;
static gchar     *losung_simple_text;
static guint      losung_simple_text_len;

static Source    *local_collections;
static gchar     *lang = NULL;
static GHashTable*lang_translations;
static Source    *server_list = NULL;
static gboolean   dark_mode;

#if ! GTK_CHECK_VERSION(2,20,0)
	/* compatibility with older of Gtk+ */
	#define gtk_widget_get_realized GTK_WIDGET_REALIZED
#endif


/****************************\
      Function prototypes
\****************************/


static GHashTable *init_languages       (void);

static void        show_text            (GDateTime *date);

static gint handle_local_options (GtkApplication *app, GVariantDict *options, gpointer data);
static void activate             (GtkApplication *app, gpointer data);
static void activate_quit        (GSimpleAction  *action,
                                  GVariant       *parameter,
                                  gpointer        app);
static void add_lang_cb          (GtkWidget *w,   gpointer data);
void about_cb             (GtkWidget *w,   gpointer data);
void calendar_cb          (GtkWidget *w,   gpointer data);
void calendar_select_cb   (GtkWidget *cal, gpointer data);
static GString* create_years_string (gchar *lang);
static void sources_changed      (GtkWidget *w,   gpointer data);
static void language_changed     (GtkWidget *w,   gpointer data);
void lang_manager_cb      (GtkWidget *w,   gpointer data);
void next_day_cb          (GtkWidget *w,   gpointer data);
void next_month_cb        (GtkWidget *w,   gpointer data);
static void no_languages_cb      (GtkWidget *w,   gpointer data);
void prev_day_cb          (GtkWidget *w,   gpointer data);
void prev_month_cb        (GtkWidget *w,   gpointer data);
void property_cb          (GtkWidget *w,   gpointer data);
void today_cb             (GtkWidget *w,   gpointer data);
static void update_language_store();
static gboolean check_new_date_cb (gpointer data);


void show_warning_cb        (GtkWidget *w,        gpointer data);
void autostart_cb           (GtkWidget *w,        gpointer data);
void autostart_once_cb      (GtkWidget *w,        gpointer data);
void font_sel_cb            (GtkWidget *button,   gpointer data);
void lang_changed_cb        (GtkWidget *item,     gpointer data);
void proxy_toggled_cb       (GtkWidget *toggle,   gpointer data);
void proxy_changed_cb       (GtkWidget *toggle,   gpointer data);
void sword_cb               (GtkWidget *toggle,   gpointer data);

gchar* get_button_icon_name (gpointer   x,        gpointer basename);


static void
activate_quit (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       app)
{
        g_application_quit (G_APPLICATION (app));
}


static void
do_copy (GtkWidget *w)
{
        GdkClipboard* clipboard;

        clipboard = gdk_display_get_clipboard (gtk_widget_get_display (w));
        // clipboard = gdk_display_get_primary_clipboard ();
        gdk_clipboard_set_text (clipboard, losung_simple_text);
} /* do_copy */


static void
activate_copy (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       app)
{
        GList *list = gtk_application_get_windows (GTK_APPLICATION (app));
        if (! list) {
                return;
        }

        do_copy (GTK_WIDGET (list->data));
} /* activate_copy */


G_MODULE_EXPORT void
copy_cb (GtkWidget *w, gpointer data)
{
        do_copy (w);
} /* copy_cb */


static gint
handle_local_options (GtkApplication *app,
                      GVariantDict   *options,
                      gpointer        user_data)
{
        date = g_date_time_new_now_local ();

        if (once) {
		GDateTime *last_time = get_last_usage ();
		if (last_time && g_date_time_compare (last_time, date) >= 0) {
			return EXIT_SUCCESS;
                }
        }
        set_last_usage (date);

        return -1;
} /* handle_local_options */


static void
set_headerbar_button (GtkBuilder *builder,
                      const char *name)
{
        GObject *button = gtk_builder_get_object (builder, name);
        gtk_widget_add_css_class (GTK_WIDGET (button), "flat");
} /* set_headerbar_button */


#define DX 0 // index in GArray∗ scroll_collector for x value
#define DY 1 // index in GArray∗ scroll_collector for y value
#define MODIFIER 2 // index in GArray∗ scroll_collector for current modifier key

gboolean
scroll_cb (GtkEventControllerScroll *self,
           gdouble                   dx,
           gdouble                   dy,
           gpointer                  user_data)
{
        GArray* scroll_collector = user_data;
        gdouble *x = &g_array_index (scroll_collector, gdouble, DX);
        *x = *x + dx;
        gdouble *y = &g_array_index (scroll_collector, gdouble, DY);
        *y = *y + dy;
        // g_message ("%f - %f  -->  %f - %f", dx, dy, *x, *y);
        return TRUE;
} /* scroll_cb */


void
scroll_end_cb (GtkEventControllerScroll* self,
               gpointer user_data)
{
        GArray* scroll_collector = user_data;
        gdouble *x   = &g_array_index (scroll_collector, gdouble, DX);
        gdouble *y   = &g_array_index (scroll_collector, gdouble, DY);
        gdouble *mod = &g_array_index (scroll_collector, gdouble, MODIFIER);

        // g_message ("%f - %f", *x, *y);
        if (*x + *y > 0) {
                if (*mod == GDK_CONTROL_MASK) {
                        next_month_cb (NULL, NULL);
                } else {
                        next_day_cb (NULL, NULL);
                }
        } else if (*x + *y < 0) {
                if (*mod == GDK_CONTROL_MASK) {
                        prev_month_cb (NULL, NULL);
                } else {
                        prev_day_cb (NULL, NULL);
                }
        }

        *x = .0;
        *y = .0;
} /* scroll_end_cb */


gboolean
key_pressed_cb (GtkEventControllerKey* self,
                guint keyval,
                guint keycode,
                GdkModifierType state,
                gpointer user_data)
{
        // g_message ("%d %d %d (%d)", keyval, keycode, state, GDK_CONTROL_MASK);
        if (state == GDK_CONTROL_MASK) {
                if (keycode == 114 /* Right */ || keycode == 116 /* Down */) {
                        next_month_cb (NULL, NULL);
                        return TRUE;
                } else if (keycode == 113 /* Left */ || keycode == 111 /* Up */) {
                        prev_month_cb (NULL, NULL);
                        return TRUE;
                }
        } else {
                if (keycode == 114 /* Right */ || keycode == 116 /* Down */) {
                        next_day_cb (NULL, NULL);
                        return TRUE;
                } else if (keycode == 113 /* Left */ || keycode == 111 /* Up */) {
                        prev_day_cb (NULL, NULL);
                        return TRUE;
                }
        }
        if (keycode == 37) { // Control
                GArray* scroll_collector = user_data;
                gdouble *mod = &g_array_index (scroll_collector, gdouble, MODIFIER);
                *mod = (gdouble) GDK_CONTROL_MASK;
        }
        return FALSE;
} /* key_pressed_cb */


void
key_released_cb (GtkEventControllerKey* self,
                guint keyval,
                guint keycode,
                GdkModifierType state,
                gpointer user_data)
{
        if (keycode == 37) { // Control
                GArray* scroll_collector = user_data;
                gdouble *mod = &g_array_index (scroll_collector, gdouble, MODIFIER);
                *mod = .0;
        }
} /*key_released_cb*/


static void
notify_dark_cb (GtkWidget *widget, gpointer data)
{
        g_message("notify dark mode");
}


static void
activate (GtkApplication *app,
          gpointer        user_data)
{
        guint timeout;

        AdwStyleManager *manager = adw_style_manager_get_default ();
        dark_mode = adw_style_manager_get_dark (manager);

        g_signal_connect (manager, "notify::dark", G_CALLBACK (notify_dark_cb), NULL);

        lang_translations = init_languages ();
        local_collections = get_local_collections ();

        lang = get_language ();
        if (lang == NULL) {
                /* should be translated to corresponding language, e.g. 'de' */
                lang = _("en");

                /* is requested language available,
                   if not use first available language instead */

                if (local_collections->languages->len > 0 &&
                    ! source_get_collections (local_collections, lang))
                {
                        lang = VC (g_ptr_array_index
                        	(local_collections->languages, 0))->language;
                }
        }

        g_message ("Choosen language: %s", lang);
        calendar_close = is_calender_double_click ();
        font = get_font ();
        show_sword = show_sword_new = is_link_sword ();

        GError *error = NULL;
        GtkBuilder *builder = gtk_builder_new ();
        gtk_builder_set_translation_domain (builder, PACKAGE);
        gtk_builder_add_from_resource (builder, "/org/godehardt/glosung/ui/glosung.ui", &error);

        /* Connect signal handlers to the constructed widgets. */
        GObject *window = gtk_builder_get_object (builder, "window");
        gtk_window_set_application (GTK_WINDOW (window), app);

        label [TITLE] = GTK_WIDGET (gtk_builder_get_object (builder, "date"));

        label [OT_TEXT] = GTK_WIDGET (gtk_builder_get_object (builder, "losung"));
        gtk_widget_add_css_class (GTK_WIDGET (label [OT_TEXT]), "spaceabove");
        label [OT_LOC] = GTK_WIDGET (gtk_builder_get_object (builder, "losung_address"));
        label [OT_LOC_SWORD] = GTK_WIDGET (gtk_builder_get_object (builder, "losung_address_link"));

        label [NT_TEXT] = GTK_WIDGET (gtk_builder_get_object (builder, "lehrtext"));
        gtk_widget_add_css_class (GTK_WIDGET (label [NT_TEXT]), "spaceabove");
        label [NT_LOC] = GTK_WIDGET (gtk_builder_get_object (builder, "lehrtext_address"));
        label [NT_LOC_SWORD] = GTK_WIDGET (gtk_builder_get_object (builder, "lehrtext_address_link"));

        label [READING] = GTK_WIDGET (gtk_builder_get_object (builder, "reading"));
        gtk_widget_add_css_class (GTK_WIDGET (label [READING]), "spaceabove");

        if (font != NULL) {
                PangoAttrList *const attrs = pango_attr_list_new ();
                PangoFontDescription *font_desc = pango_font_description_from_string (font);
                pango_attr_list_insert (attrs, pango_attr_font_desc_new (font_desc));
                for (gint i = 0; i < NUMBER_OF_LABELS; i++) {
                        if (i != OT_LOC_SWORD && i != NT_LOC_SWORD) {
                                gtk_label_set_attributes (GTK_LABEL (label[i]), attrs);
                        }
                }
                pango_attr_list_unref (attrs);
                pango_font_description_free (font_desc);
        }

        set_headerbar_button (builder, "languages");
        set_headerbar_button (builder, "properties");
        set_headerbar_button (builder, "about");
        set_headerbar_button (builder, "previous_month");
        set_headerbar_button (builder, "previous_day");
        set_headerbar_button (builder, "today");
        set_headerbar_button (builder, "next_day");
        set_headerbar_button (builder, "next_month");
        set_headerbar_button (builder, "calendar");
        set_headerbar_button (builder, "copy");

        /* start timeout for detecting date change */
	timeout = g_timeout_add_seconds (UPDATE_TIME, check_new_date_cb, NULL);
        /* call the date check with not NULL as arg to initialize */
        check_new_date_cb (&timeout);

        GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (window));
        GtkCssProvider *provider = gtk_css_provider_new ();
        //gtk_css_provider_load_from_data (provider, "#content {margin: 20px;} button.minimize {display: none; margin: 0px; background-color: red;}", -1);
        // gchar *css_file = find_ui_file ("style.css");
        // gtk_css_provider_load_from_path ( provider, css_file);
        // g_free (css_file);
        gtk_css_provider_load_from_resource (provider, "/org/godehardt/glosung/ui/style.css");
        gtk_style_context_add_provider_for_display (display, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        gtk_widget_show (GTK_WIDGET (window));
        // gtk_window_set_default_icon_from_file
        //         (PACKAGE_PIXMAPS_DIR "glosung.png", NULL);

        if (local_collections->languages->len == 0) {
                GtkWidget *error_dialog = gtk_message_dialog_new
                        (GTK_WINDOW (app), GTK_DIALOG_DESTROY_WITH_PARENT,
                         GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                         _("No text files found!\n"
                           "Please install your prefered languages."));
                g_signal_connect (G_OBJECT (error_dialog), "response",
                                  G_CALLBACK (no_languages_cb), NULL);
                               // G_CALLBACK (gtk_widget_destroy), NULL);
                               // G_CALLBACK (lang_manager_cb), NULL);
                               // CALLBACK (gtk_main_quit), NULL);
                gtk_widget_show (error_dialog);
        } else {
                show_text (date);
        }

        /* We don't need the builder any more */
        g_object_unref (builder);

        GArray *scroll_collector = g_array_sized_new (FALSE, FALSE, sizeof (gdouble), 3);
        gdouble zero = .0;
        g_array_append_val (scroll_collector, zero);
        g_array_append_val (scroll_collector, zero);
        g_array_append_val (scroll_collector, zero);
        GtkEventController* scroll = gtk_event_controller_scroll_new (GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES | GTK_EVENT_CONTROLLER_SCROLL_DISCRETE);
        g_signal_connect (G_OBJECT (scroll), "scroll", G_CALLBACK (scroll_cb), scroll_collector);
        g_signal_connect (G_OBJECT (scroll), "scroll_end", G_CALLBACK (scroll_end_cb), scroll_collector);
        gtk_widget_add_controller (GTK_WIDGET (window), scroll);

        GtkEventController* key = gtk_event_controller_key_new ();
        g_signal_connect (G_OBJECT (key), "key-pressed", G_CALLBACK (key_pressed_cb), scroll_collector);
        g_signal_connect (G_OBJECT (key), "key-released", G_CALLBACK (key_released_cb), scroll_collector);
        gtk_widget_add_controller (GTK_WIDGET (window), key);

        const GActionEntry app_entries[] =
        {
                { "quit", activate_quit},
                { "copy", activate_copy }
        };
        const char *quit_accels[2] = { "<Control>q", NULL };
        const char *copy_accels[2] = { "<Control>c", NULL };
        g_action_map_add_action_entries (G_ACTION_MAP (app),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   app);
        gtk_application_set_accels_for_action (GTK_APPLICATION (app),
                                         "app.quit",
                                         quit_accels);
        gtk_application_set_accels_for_action (GTK_APPLICATION (app),
                                         "app.copy",
                                         copy_accels);
} /* activate */


/*
 * the one and only main function :-)
 */
int main (int argc, char **argv)
{
        /* Initialize the i18n stuff */
        // setlocale (LC_ALL, "");
        bindtextdomain (PACKAGE, "/usr/share/locale");
        bind_textdomain_codeset (PACKAGE, "UTF-8");
        textdomain (PACKAGE);

        GtkApplication *app = gtk_application_new ("org.godehardt.glosung", G_APPLICATION_FLAGS_NONE);
        g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
        g_signal_connect (app, "handle-local-options", G_CALLBACK (handle_local_options), NULL);

        GOptionEntry options[] = {
                { "once", '1', 0, G_OPTION_ARG_NONE, &once,
                  N_("Start only once a day"), NULL },
                { NULL }
        };
        g_application_add_main_option_entries (G_APPLICATION (app), options);

        int status = g_application_run (G_APPLICATION (app), argc, argv);
        g_object_unref (app);

        return status;
} /* main */


static GHashTable *
init_languages (void)
{
        GHashTable *ht = g_hash_table_new (g_str_hash, g_str_equal);
        g_hash_table_insert (ht, "af",     ("Afrikaans"));
        g_hash_table_insert (ht, "ar",     ("العربية"));
        g_hash_table_insert (ht, "bg",     ("Български"));
        g_hash_table_insert (ht, "cs",     ("Čeština"));
        g_hash_table_insert (ht, "de",     ("Deutsch"));
        g_hash_table_insert (ht, "el",     ("Ελληνικά"));
        g_hash_table_insert (ht, "en",     ("English"));
        g_hash_table_insert (ht, "es",     ("Español"));
        g_hash_table_insert (ht, "et",     ("Eesti"));
        g_hash_table_insert (ht, "fr",     ("Français"));
        g_hash_table_insert (ht, "he",     ("עברית‏"));
        g_hash_table_insert (ht, "hu",     ("Magyar"));
        g_hash_table_insert (ht, "it",     ("Italiano"));
        g_hash_table_insert (ht, "nl",     ("Nederlands"));
        g_hash_table_insert (ht, "no",     ("Norsk"));
        g_hash_table_insert (ht, "nb",     ("Norsk"));
        g_hash_table_insert (ht, "pt",     ("Português"));
        g_hash_table_insert (ht, "ro",     ("Română"));
        g_hash_table_insert (ht, "ru",     ("Русский"));
        g_hash_table_insert (ht, "sw",     ("Kiswahili"));
        g_hash_table_insert (ht, "ta",     ("தமிழ்"));
        g_hash_table_insert (ht, "th",     ("ไทย"));
        g_hash_table_insert (ht, "tr",     ("Türkçe"));
        g_hash_table_insert (ht, "vi",     ("Tiếng Việt"));
        g_hash_table_insert (ht, "zh",     ("中文"));
        g_hash_table_insert (ht, "zh-CN",  ("中文 (简体)"));
        g_hash_table_insert (ht, "zh-Hans",("中文 (简体)"));
        g_hash_table_insert (ht, "hans",   ("中文 (简体)"));
        g_hash_table_insert (ht, "zh-TW",  ("中文 (繁體)"));
        g_hash_table_insert (ht, "zh-Hant",("中文 (繁體)"));

        return ht;
} /* init_languages */


/*
 * display the ww for the current date.
 * must not unref new_date after
 */
static void
show_text (GDateTime *new_date)
{
        const Losung *ww;

        ww = get_orig_losung (new_date, lang);
        if (! ww) {
                ww = get_losung (new_date, lang);
                if (! ww) {
                        ww = get_the_word (new_date, lang);
                }
                if (ww) {
                	if (((strlen (ww->ot.text) > WRAP_LENGTH + 13)
                			&& ! strstr (ww->ot.text, "\n"))
                	  ||
                	    ((strlen (ww->nt.text) > WRAP_LENGTH + 13)
                			&& ! strstr (ww->nt.text, "\n")))
                	{
                        	wrap_text (ww->ot.text, WRAP_LENGTH);
                        	wrap_text (ww->nt.text, WRAP_LENGTH);
                	}
                }
        } else {
        	wrap_text (ww->ot.text, WRAP_LENGTH);
        	wrap_text (ww->nt.text, WRAP_LENGTH);
        }

        if (ww == NULL) {
                GtkWidget *error_dialog = gtk_message_dialog_new
                        (GTK_WINDOW (app), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                         GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                        _("No %s texts found for %d!"),
                        (gchar*) g_hash_table_lookup (lang_translations, lang),
                        g_date_time_get_year (new_date));
                g_signal_connect (G_OBJECT (error_dialog), "response",
                                  G_CALLBACK (no_languages_cb), NULL);

                gtk_widget_show (error_dialog);
                // FIXME: needed? core dumb
                // g_object_unref (new_date);
                return;
        }

        if (date != new_date) {
                // FIXME: needed? core dumb
                //g_object_unref (date);
                date = new_date;
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
                                  G_CALLBACK (gtk_window_destroy), NULL);

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
                gtk_widget_hide (label [READING]);
        }

        for (gint i = 0; i < NUMBER_OF_LABELS; i++) {
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
 * callback function that displays the about dialog.
 */
G_MODULE_EXPORT void
about_cb (GtkWidget *w, gpointer data)
{
        about (app);
} /* about_cb */


/*
 * callback function that displays a property dialog.
 */
G_MODULE_EXPORT void
property_cb (GtkWidget *w, gpointer data)
{
        if (new_font != NULL) {
                g_free (new_font);
                new_font = NULL;
        }

        GtkBuilder* builder = gtk_builder_new ();
        gtk_builder_set_translation_domain (builder, PACKAGE);
        gboolean result = gtk_builder_add_from_resource (builder, "/org/godehardt/glosung/ui/preferences.ui", NULL);
        if (! result) {
                g_message ("Error while loading UI definition file");
                return;
        }

        GtkWidget *preferences = GTK_WIDGET
                (gtk_builder_get_object (builder, "preferences_dialog"));
        GtkWidget *proxy_checkbox = GTK_WIDGET
                (gtk_builder_get_object (builder, "proxy_checkbox"));
        GtkWidget *proxy_entry = GTK_WIDGET
                (gtk_builder_get_object (builder, "proxy_entry"));
        GtkWidget *herrnhut_url_entry = GTK_WIDGET
                (gtk_builder_get_object (builder, "herrnhut_url_entry"));

        GtkWidget *combo = GTK_WIDGET
                (gtk_builder_get_object (builder, "language_combo"));
        for (gint i = 0; i < (local_collections->languages)->len; i++) {
                gchar *langu = g_ptr_array_index
                                (local_collections->languages, i);
                gtk_combo_box_text_append_text
                        (GTK_COMBO_BOX_TEXT (combo),
                                g_hash_table_lookup (lang_translations, langu));
                if (strcmp (lang, langu) == 0) {
                        gtk_combo_box_set_active
                                (GTK_COMBO_BOX (combo), i);
                }
        }

        if (is_proxy_in_use ()) {
                gtk_check_button_set_active
                        (GTK_CHECK_BUTTON (proxy_checkbox), TRUE);
                gtk_widget_set_sensitive (proxy_entry, TRUE);
        }
        gchar *proxy = get_proxy ();
        if (proxy && strlen (proxy) > 0) {
                gtk_editable_set_text (GTK_EDITABLE (proxy_entry), proxy);
        }

        if (font != NULL) {
                gtk_font_chooser_set_font (GTK_FONT_CHOOSER
                        (gtk_builder_get_object (builder, "fontbutton")),
                        font);
        }
        GLosungAutostartType autostart = is_in_autostart ();
        if (autostart != GLOSUNG_NO_AUTOSTART) {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                        (gtk_builder_get_object (builder,
                                                "autostart_checkbox")),
                        TRUE);
        }
        if (autostart == GLOSUNG_AUTOSTART_ONCE) {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                        (gtk_builder_get_object (builder,
                                                "autostart_once_checkbox")),
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
        gchar *herrnhut_url = get_herrnhuter_url ();
        if (! herrnhut_url) {
                gtk_entry_set_placeholder_text (GTK_ENTRY (herrnhut_url_entry), "https://www.losungen.de/fileadmin/media-losungen/download/Losung_%d_XML.zip");
        } else {
                gtk_editable_set_text (GTK_EDITABLE (herrnhut_url_entry), herrnhut_url);
        }

        gtk_widget_show (preferences);
} /* property_cb */


/*
 * callback function for preferences dialog.
 */
G_MODULE_EXPORT void
proxy_toggled_cb (GtkWidget *toggle, gpointer proxy_entry)
{
        gboolean on = gtk_check_button_get_active (GTK_CHECK_BUTTON (toggle));

        gtk_widget_set_sensitive (GTK_WIDGET (proxy_entry), on);

        set_proxy_in_use (on);
} /* proxy_toggled_cb */


/*
 * callback function for preferences dialog.
 */
G_MODULE_EXPORT void
proxy_changed_cb (GtkWidget *entry, gpointer data)
{
        const gchar *proxy = gtk_editable_get_text (GTK_EDITABLE (entry));
        set_proxy (proxy);
} /* proxy_changed_cb */


/*
 * callback function for preferences dialog.
 */
G_MODULE_EXPORT void
herrnhut_url_changed_cb (GtkWidget *entry, gpointer data)
{
        const gchar *url = gtk_editable_get_text (GTK_EDITABLE (entry));
        set_herrnhuter_url (url);
} /* herrnhut_url_changed_cb */


/*
 * callback function for options menu.
 */
G_MODULE_EXPORT void
lang_changed_cb (GtkWidget *combo, gpointer data)
{
        gint num = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));
        new_lang = (gchar*) g_ptr_array_index (local_collections->languages, num);
        if (new_lang != NULL) {
                lang = new_lang;
                new_lang = NULL;
                set_language (lang);
                show_text (date);
        }
} /* lang_changed_cb */


/*
 * callback function when font is selected.
 */
G_MODULE_EXPORT void
font_sel_cb (GtkWidget *gfc, gpointer data)
{
        // TODO check wether gtk_font_chooser_get_font_desc is better
        const gchar* font_name = gtk_font_chooser_get_font
                (GTK_FONT_CHOOSER (gfc));
        if (font_name != NULL
            && (font == NULL || strcmp (font, font_name) != 0))
        {
                if (new_font != NULL) {
                        g_free (new_font);
                }
                new_font = g_strdup (font_name);
                if (new_font != NULL) {
                        PangoFontDescription *font_desc;

                        if (font != NULL) {
                                g_free (font);
                        }
                        font = new_font;
                        new_font = NULL;
                        set_font (font);

                        PangoAttrList *const attrs = pango_attr_list_new();
                        font_desc = pango_font_description_from_string (font);
                        pango_attr_list_insert(attrs, pango_attr_font_desc_new(font_desc));
                        for (gint i = 0; i < NUMBER_OF_LABELS; i++) {
                                if (i != OT_LOC_SWORD && i != NT_LOC_SWORD) {
                                        gtk_label_set_attributes (GTK_LABEL (label [i]), attrs);
                                }
                        }
                        pango_attr_list_unref(attrs);
                        pango_font_description_free (font_desc);
                }
        }
} /* font_sel_cb */


/*
 * callback function for autostart changes.
 */
G_MODULE_EXPORT void
autostart_cb (GtkWidget *toggle, gpointer data) {
        gboolean autostart =
                gtk_check_button_get_active (GTK_CHECK_BUTTON (toggle));
        if (autostart) {
                /* TODO handle return type */
                add_to_autostart (gtk_check_button_get_active (GTK_CHECK_BUTTON (data)));
        } else {
                /* TODO handle return type */
                remove_from_autostart ();
        }
        gtk_widget_set_sensitive (GTK_WIDGET (data), autostart);
} /* autostart_cb */


/*
 * callback function for autostart changes.
 */
G_MODULE_EXPORT void
autostart_once_cb (GtkWidget *toggle, gpointer data) {
        gboolean once = gtk_check_button_get_active (GTK_CHECK_BUTTON (toggle));
        remove_from_autostart ();

        /* TODO handle return type */
        add_to_autostart (once);
} /* autostart_cb */


/*
 * callback function for gnomwsword option change.
 */
G_MODULE_EXPORT void
sword_cb (GtkWidget *toggle, gpointer data)
{
        show_sword = gtk_check_button_get_active (GTK_CHECK_BUTTON (toggle));

#ifdef VERSE_LINK
        set_link_sword (show_sword);
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
#endif
} /* sword_cb */


/*
 * callback function that displays a calendar.
 */
G_MODULE_EXPORT void
calendar_cb (GtkWidget *w, gpointer data)
{
        GtkWidget *dialog = gtk_dialog_new ();
        gtk_window_set_title (GTK_WINDOW (dialog), _("Calendar"));
        gtk_dialog_add_action_widget (
                GTK_DIALOG (dialog),
                gtk_button_new_with_label ("_Close"),
                GTK_RESPONSE_CLOSE);
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (app));

        calendar = gtk_calendar_new ();
        gtk_calendar_set_show_day_names (GTK_CALENDAR (calendar), TRUE);
        gtk_calendar_set_show_heading (GTK_CALENDAR (calendar), TRUE);
        gtk_calendar_set_show_week_numbers (GTK_CALENDAR (calendar), TRUE);
        gtk_calendar_select_day      (GTK_CALENDAR (calendar), date);
        gtk_box_append (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), calendar);
        g_signal_connect (G_OBJECT (dialog), "response",
                                G_CALLBACK (gtk_window_destroy), &dialog);
        g_signal_connect (G_OBJECT (calendar),
                                "day_selected", // day_selected_double_click
                                G_CALLBACK (calendar_select_cb), dialog);

        gtk_widget_show (dialog);
} /* calendar_cb */


/*
 * callback function that computes Losung for the current date.
 */
G_MODULE_EXPORT void
today_cb (GtkWidget *w, gpointer data)
{
        // FIXME: needed? core dumb
        // g_object_unref (date);
        date = g_date_time_new_now_local ();

        show_text (date);
        if (calendar != NULL && GTK_IS_CALENDAR (calendar)) {
                gtk_calendar_select_day (GTK_CALENDAR (calendar), date);
        }
} /* today_cb */


/*
 * callback function that increases the day.
 */
G_MODULE_EXPORT void
next_day_cb (GtkWidget *w, gpointer data)
{
        GDateTime *new_date = g_date_time_add_days (date, 1);
        show_text (new_date);
        if (calendar != NULL && GTK_IS_CALENDAR (calendar)) {
                gtk_calendar_select_day (GTK_CALENDAR (calendar), new_date);
        }
} /* next_day_cb */


/*
 * callback function that decreases the day.
 */
G_MODULE_EXPORT void
prev_day_cb (GtkWidget *w, gpointer data)
{
        GDateTime *new_date = g_date_time_add_days (date, -1);
        show_text (new_date);
        if (calendar != NULL && GTK_IS_CALENDAR (calendar)) {
                gtk_calendar_select_day (GTK_CALENDAR (calendar), new_date);
        }
} /* prev_day_cb */


/*
 * callback function  that increases the month.
 */
G_MODULE_EXPORT void
next_month_cb (GtkWidget *w, gpointer data)
{
        GDateTime *new_date = g_date_time_add_months (date, 1);
        show_text (new_date);
        if (calendar != NULL && GTK_IS_CALENDAR (calendar)) {
                gtk_calendar_select_day (GTK_CALENDAR (calendar), new_date);
        }
} /* next_month_cb */


/*
 * callback function that decreases the month.
 */
G_MODULE_EXPORT void
prev_month_cb (GtkWidget *w, gpointer data)
{
        GDateTime *new_date = g_date_time_add_months (date, -1);
        show_text (new_date);
        if (calendar != NULL && GTK_IS_CALENDAR (calendar)) {
                gtk_calendar_select_day (GTK_CALENDAR (calendar), new_date);
        }
} /* prev_month_cb */


/*
 * callback function for the calendar widget.
 */
G_MODULE_EXPORT void
calendar_select_cb (GtkWidget *calendar, gpointer data)
{
        show_text (gtk_calendar_get_date (GTK_CALENDAR (calendar)));

//        if (calendar_close) {
//                gtk_window_destroy (GTK_WINDOW (data));
//        }
} /* calendar_select_cb */







static GtkComboBox     *lang_combo;
static GtkComboBoxText *year_combo;
static GtkWidget       *download_button;
static GtkListStore    *store;


static void
lang_manager_response (GtkWidget *w, gpointer data)
{
        gint response = GPOINTER_TO_INT (data);
        if (response == GTK_RESPONSE_NONE) {
                gtk_window_destroy (GTK_WINDOW (w));
        } else if (response == GTK_RESPONSE_DELETE_EVENT) {
                // Window close button.  No reaction needed?!?
        } else if (response == 1) {
                add_lang_cb (w, w);
        } else if (response == 2) {
                g_print ("download %d\n", response);

                gint index = gtk_combo_box_get_active (GTK_COMBO_BOX (lang_combo));
                if (index == -1) {
                	return;
                }
                GPtrArray *langs = source_get_languages (server_list);
                if (langs->len > 0 && index > 0) {
                	index--;
                }
        	gchar *langu = g_ptr_array_index (langs, index);

                GPtrArray *vc_s = source_get_collections (server_list, langu);
                gint i = gtk_combo_box_get_active (GTK_COMBO_BOX (year_combo));
		guint year = VC (g_ptr_array_index (vc_s, i))->year;

		if (! is_hide_warning ()) {
                        GtkBuilder* builder = gtk_builder_new ();
                        gtk_builder_set_translation_domain (builder, PACKAGE);
                        gtk_builder_add_from_resource (builder, "/org/godehardt/glosung/ui/warning_dialog.ui", NULL);
			GtkDialog *warning = GTK_DIALOG
			   (gtk_builder_get_object (builder, "warning_dialog"));
                        gtk_widget_show (GTK_WIDGET (warning));
/*
			gint res = gtk_dialog_run (warning);
			gtk_window_destroy (GTK_WINDOW (warning));
//		        gtk_window_destroy (GTK_WINDOW (dialog));
		        if (res == GTK_RESPONSE_CANCEL) {
		        	return;
		        }
*/
		}

	        int error = download (server_list, langu, year);
                if (error) {
                	GtkWidget *msg = gtk_message_dialog_new
                	     (GTK_WINDOW (app), GTK_DIALOG_DESTROY_WITH_PARENT,
                	      GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s (%d)",
                	      get_last_error_message (), error);
                	gtk_widget_show (GTK_WIDGET (msg));
//                 	gtk_window_destroy (GTK_WINDOW (msg));
                	return;
                }

                source_add_collection (local_collections, langu, year);
		source_finialize      (local_collections);

		update_language_store ();
		if (local_collections->languages->len == 1) {
			lang = langu;
			set_language (lang);
		}
                gtk_window_destroy (GTK_WINDOW (w));
                show_text (date);
        } else {
                g_print ("%d\n", response);
        }
}


G_MODULE_EXPORT void
lang_manager_cb (GtkWidget *w, gpointer data)
{
        GtkWidget    *dialog;

        GtkBuilder* builder = gtk_builder_new ();
        gtk_builder_set_translation_domain (builder, PACKAGE);
        gboolean result = gtk_builder_add_from_resource (builder, "/org/godehardt/glosung/ui/language_manager.ui", NULL);
        if (! result) {
                g_message ("Error while loading UI definition file");
                return;
        }
        // gtk_builder_connect_signals (builder, NULL);
        dialog = GTK_WIDGET
                (gtk_builder_get_object (builder, "language_manager_dialog"));

        GtkScrolledWindow *scroll = GTK_SCROLLED_WINDOW
                (gtk_builder_get_object (builder, "installed_lang_scroll"));

        store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
        update_language_store (store);

        GtkWidget *list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

        GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes
                (NULL, renderer, "text", 0, NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes
                (NULL, renderer, "text", 1, NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
        gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), FALSE);

        gtk_scrolled_window_set_child (scroll, list);

        g_signal_connect (G_OBJECT (dialog), "response",
                          G_CALLBACK (lang_manager_response), NULL);

        gtk_widget_show (dialog);
} /* lang_manager_cb */


static void
add_lang_cb (GtkWidget *w, gpointer data)
{
        GtkWidget    *dialog;

        GtkBuilder* builder = gtk_builder_new ();
        gtk_builder_set_translation_domain (builder, PACKAGE);
        gboolean result = gtk_builder_add_from_resource (builder, "/org/godehardt/glosung/ui/add_language.ui", NULL);
        if (! result) {
                g_message ("Error while loading UI definition file");
                return;
        }
        // gtk_builder_connect_signals (builder, NULL);
        dialog = GTK_WIDGET
                (gtk_builder_get_object (builder, "add_language_dialog"));

        GtkBox *source_frame = GTK_BOX
                (gtk_builder_get_object (builder, "source_frame"));
        GtkComboBoxText *source_combo =
		GTK_COMBO_BOX_TEXT (gtk_combo_box_text_new ());
        gtk_box_append (source_frame, GTK_WIDGET (source_combo));
        gtk_combo_box_text_append_text (source_combo, "");

        GPtrArray *sources = get_sources ();
        for (gint i = 0; i < sources->len; i++) {
                Source *cs = (Source*) g_ptr_array_index (sources, i);
                if (cs->type != SOURCE_LOCAL) {
                        gtk_combo_box_text_append_text (source_combo, cs->name);
                }
        }
        gtk_combo_box_set_active (GTK_COMBO_BOX (source_combo), 0);

        GtkBox *lang_frame = GTK_BOX
                (gtk_builder_get_object (builder, "lang_frame"));
        GtkComboBoxText *lang_combo =
		GTK_COMBO_BOX_TEXT (gtk_combo_box_text_new ());
        gtk_box_append (lang_frame, GTK_WIDGET (lang_combo));
        gtk_widget_set_sensitive (GTK_WIDGET (lang_combo), FALSE);

        GtkBox *year_frame = GTK_BOX
                (gtk_builder_get_object (builder, "year_frame"));
        download_button = GTK_WIDGET
                (gtk_builder_get_object (builder, "button_download"));
	year_combo = GTK_COMBO_BOX_TEXT (gtk_combo_box_text_new ());
        gtk_box_append (year_frame, GTK_WIDGET (year_combo));
        gtk_widget_set_sensitive (GTK_WIDGET (year_combo), FALSE);

        gtk_combo_box_set_active (GTK_COMBO_BOX (year_combo), 0);

        g_signal_connect (G_OBJECT (source_combo), "changed",
                          G_CALLBACK (sources_changed), lang_combo);
        g_signal_connect (G_OBJECT (lang_combo), "changed",
                          G_CALLBACK (language_changed), year_combo);
        g_signal_connect (G_OBJECT (dialog), "response",
                          G_CALLBACK (lang_manager_response), NULL);
        gtk_widget_show (dialog);

/* FIXME gtk4 use callback
        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
                // gchar *langu = "de";
       }
        gtk_window_destroy (GTK_WINDOW (dialog));
        */
} /* add_lang_cb */


G_MODULE_EXPORT void
show_warning_cb (GtkWidget *w, gpointer data)
{
        gboolean on = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));

	set_hide_warning (on);
} /* show_warning_cb */


static void
no_languages_cb (GtkWidget *w, gpointer data)
{
        gtk_window_destroy (GTK_WINDOW (w));
        lang_manager_cb (app, data);
} /* no_languages_cb */


static void
sources_changed (GtkWidget *w, gpointer data)
{
        GtkComboBoxText *lang_combo = GTK_COMBO_BOX_TEXT (data);
        while (gtk_combo_box_get_active (GTK_COMBO_BOX (lang_combo)) != -1) {
                gtk_combo_box_text_remove (lang_combo, 0);
                gtk_combo_box_set_active (GTK_COMBO_BOX (lang_combo), 0);
        }

        gint index = gtk_combo_box_get_active (GTK_COMBO_BOX (w));
        if (index == 0) {
                gtk_widget_set_sensitive (GTK_WIDGET (lang_combo), FALSE);
                gtk_widget_set_sensitive (GTK_WIDGET (year_combo), FALSE);
                gtk_widget_set_sensitive (GTK_WIDGET (download_button), FALSE);
                return;
        }

        server_list = g_ptr_array_index (get_sources (), index);
        GPtrArray *langs = source_get_languages (server_list);

        if (! langs) {
        	return;
        }
        if (langs->len > 1) {
                gtk_combo_box_text_append_text (lang_combo, "");
        }
        for (gint i = 0; i < langs->len; i++) {
        	gchar *lang_code = (gchar*) g_ptr_array_index (langs, i);
        	gchar *langu = (gchar*)
        		g_hash_table_lookup (lang_translations, lang_code);
        	if (! langu) {
        		// continue;
        		langu = lang_code;
        	}
                gtk_combo_box_text_append_text (lang_combo, langu);
        }
        gtk_combo_box_set_active (GTK_COMBO_BOX (lang_combo), 0);
        gtk_widget_set_sensitive (GTK_WIDGET (lang_combo), TRUE);
} /* sources_changed */


static void
language_changed (GtkWidget *w, gpointer data)
{
        GtkComboBoxText *year_combo = GTK_COMBO_BOX_TEXT (data);
        while (gtk_combo_box_get_active  (GTK_COMBO_BOX (year_combo)) != -1) {
               gtk_combo_box_text_remove (year_combo, 0);
               gtk_combo_box_set_active  (GTK_COMBO_BOX (year_combo), 0);
        }

        gint index  = gtk_combo_box_get_active (GTK_COMBO_BOX (w));
        if (index == -1) {
        	return;
        }
        gchar *text = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (w));
        if (index == 0 && g_str_equal ("", text)) {
                gtk_widget_set_sensitive (GTK_WIDGET (year_combo), FALSE);
                gtk_widget_set_sensitive (GTK_WIDGET (download_button), FALSE);
                return;
        }

        GPtrArray *langs = source_get_languages (server_list);
        gchar *langu;
        if (langs->len > 0 && index > 0) {
        	index--;
        }
	langu = g_ptr_array_index (langs, index);
        GPtrArray *vc_s = source_get_collections (server_list, langu);
        for (gint i = 0; i < vc_s->len; i++) {
                gchar *year = g_strdup_printf
                	("%d", VC (g_ptr_array_index (vc_s, i))->year);
                gtk_combo_box_text_append_text (year_combo, year);
        }
        gtk_combo_box_set_active (GTK_COMBO_BOX (year_combo), 0);
        gtk_widget_set_sensitive (GTK_WIDGET (year_combo), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (download_button), TRUE);
} /* language_changed */


static GString*
create_years_string (gchar *langu)
{
        GPtrArray *years = source_get_collections (local_collections, langu);
        GString *result = g_string_new ("");
        g_string_printf (result, "%d",
                         VC (g_ptr_array_index (years, 0))->year);

        for (gint i = 1; i < years->len; i++) {
                g_string_append_printf
                        (result, ", %d",
                         VC (g_ptr_array_index (years, i))->year);
        }

        return result;
} /* create_years_string */


static void
update_language_store ()
{
        GtkTreeIter   iter1;

        gtk_list_store_clear (store);
        for (gint i = 0; i < local_collections->languages->len; i++) {
                gtk_list_store_append (store, &iter1);
                gchar *langu = g_ptr_array_index (local_collections->languages, i);
                GString *years = create_years_string (langu);
                gtk_list_store_set (store, &iter1,
                         0, g_hash_table_lookup (lang_translations, langu),
                         1, years->str, -1);
                g_string_free (years, TRUE);
        }
} /* update_language_store */


static gboolean
check_new_date_cb (gpointer first)
{
        /* check for current date. If it is different than last time,
         * call the today-button callback to display the corresponding text
         */
        static GDate last_date;
        GDate        now_date;
        time_t       t;
        struct tm    now;

        t = time (NULL);
        now = *localtime (&t);
        g_date_set_dmy (&now_date, now.tm_mday,
                now.tm_mon + 1, now.tm_year + 1900);
        if (first) {
                g_date_set_dmy (&last_date, now.tm_mday,
                        now.tm_mon + 1, now.tm_year + 1900);
        }
        if (g_date_get_day (&last_date)   != g_date_get_day (&now_date)
         || g_date_get_month (&last_date) != g_date_get_month (&now_date)
         || g_date_get_year (&last_date)  != g_date_get_year (&now_date))
        {
                /* date has changed */
                today_cb (NULL, NULL);

                last_date = now_date;
        }
        return TRUE;
} /* check_new_date_cb */


/*
 * function to get icon name based on dark mode, called from within glosung.ui
 */
G_MODULE_EXPORT gchar*
get_button_icon_name (gpointer x, gpointer basename)
{
        // g_message ("dark mod %d", dark_mode);
        // g_message ("%s", g_strdup_printf ("%s.png", basename));
        if (dark_mode) {
                return g_strdup_printf ("%s-white.png", (char*)basename);
        } else {
                return g_strdup_printf ("%s.png", (char*)basename);
        }
} /* get_butten_icon_name */
