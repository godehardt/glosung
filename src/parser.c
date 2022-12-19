/* parser.c
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

#include <glib/gi18n.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include "parser.h"


/****************************\
   Variables & Definitions
\****************************/

typedef enum {
STATE_START,
  STATE_LOSFILE,
    STATE_HEAD,
    STATE_YEAR,
    STATE_MONTH,
    STATE_DAY,
    STATE_LOSUNG,
      STATE_TL,
      STATE_OT,
      STATE_NT,
      STATE_TT,
      STATE_SR,
      STATE_CR,
      STATE_C,
        STATE_IL,
        STATE_S,
        STATE_L,
        STATE_SL,
          STATE_EM,
  TW_THE_WORD_FILE,
    TW_HEAD,
    TW_THE_WORD,
      TW_TITLE,
      TW_PAROL,
        TW_INTRO,
        TW_TEXT,
          TW_EM,
        TW_REF,
  LOS_DATAROOT,
  LOS_FREE_XML,
    LOS_LOSUNGEN,
      LOS_DATUM,
      LOS_WTAG,
      LOS_SONNTAG,
      LOS_LOSUNGSTEXT,
      LOS_LOSUNGSVERS,
      LOS_LEHRTEXT,
      LOS_LEHRTEXTVERS,
} State;

static gchar const * const states [] = {
"start", /* dummy string */
  "LOSFILE",
    "HEAD",
    "YEAR",
    "MONTH",
    "DAY",
    "LOSUNG",
      "TL",
      "OT",
      "NT",
      "TT",
      "SR",
      "CR",
      "C",
        "IL",
        "S",
        "L",
        "SL",
          "EM",
  "thewordfile",
    "head",
    "theword",
      "title",
      "parol",
        "intro",
        "text",
          "em",
        "ref",
  "dataroot",
  "FreeXml",
    "Losungen",
      "Datum",
      "Wtag",
      "Sonntag",
      "Losungstext",
      "Losungsvers",
      "Lehrtext",
      "Lehrtextvers",
};


/* buf_len for the string which contains the text while parsing */
#define LINE_LEN 255

static State      state;
static GSList    *stack;
static gint       depth;
static GDateTime *datum;
static gint       day;
static gboolean   found;
static gchar     *date_string;
static Losung    *ww = NULL;
static Passage   *quote = NULL;
static GString   *string;
static GString   *location;


/****************************                   \
      Function prototypes
\****************************/

static const Losung* parse   (GDateTime      *date,
                              gchar          *lang,
                              gchar          *filename);

static void start_element    (void           *ctx,
                              const xmlChar  *name,
                              const xmlChar **attrs);
static void end_element      (void           *ctx,
                              const xmlChar  *name);
static void character        (void           *ctx,
                              const xmlChar  *ch,
                              int             len);
static gboolean switch_state (const xmlChar  *name,
                              State           newState);
static void pop_state        (void);
static gchar* get_string             (GString       *string);
static gchar* get_string_with_markup (GString       *string);

static const gchar* sword_book_title (const xmlChar *book);
static const gchar* sword_book_title_for_original_losung
                                     (const gchar   *book);
static const gchar* sword_book_title_for_the_word
                                     (const xmlChar *book_number);
static gchar* sword_link_for_original_losung
                                     (const gchar   *location);
static gchar* check_file             (const gchar   *file);
static const gchar* find_the_word_file(gchar        *lang,
                                      gint           year);


/*
 * public function that frees the Losung allocated by get_losung.
 */
void
losung_free (const Losung *ww)
{
        g_free (ww->ot.say);
        g_free (ww->ot.text);
        g_free (ww->ot.location);

        g_free (ww->nt.say);
        g_free (ww->nt.text);
        g_free (ww->nt.location);

        g_free (ww->title);
        g_free (ww->comment);

        g_free ((Losung *)ww);
} /* losung_free */


/*
 * public function that parses the xml file and return required Losung.
 */
const Losung*
get_orig_losung (GDateTime *date, gchar *lang)
{
        gchar            *file;
        gchar            *filename;

        if (strcmp (lang, "de") != 0) {
                return NULL;
        }
        file = g_strdup_printf ("Losungen Free %d.xml",
                                g_date_time_get_year (date));
        filename = check_file (file);
        g_free (file);

        if (! filename) {
                return NULL;
        }

        return parse (date, lang, filename);
} /* get_orig_losung */


/*
 * public function that parses the xml file and return required Losung.
 */
const Losung*
get_losung (GDateTime *date, gchar *lang)
{
        gchar            *file;
        gchar            *filename;
        guint             year;

        year = g_date_time_get_year (date) % 100;
        if (year < 10) {
                file = g_strdup_printf ("%s_los0%d.xml", lang, year);
        } else {
                file = g_strdup_printf ("%s_los%d.xml", lang, year);
        }

        filename = check_file (file);
        g_free (file);

        if (! filename) {
                return NULL;
        }

        return parse (date, lang, filename);
} /* get_losung */


/*
 * public function that parses the xml file and return required Losung.
 */
const Losung*
get_the_word (GDateTime *date, gchar *lang)
{
        const gchar *file;
        gchar       *filename;

        file = find_the_word_file (lang, g_date_time_get_year (date));
        filename = check_file (file);

        if (! file || ! filename) {
                return NULL;
        }
        // g_message ("the word");

        return parse (date, lang, filename);
} /* get_the word */


/*
 * This function will check, if the watch word file exists in global
 * (PREFIX/share/glosung) or local (~/.glosung) directory and will
 * return the absolute filename or NULL.
 */
static gchar*
check_file (const gchar *file)
{
        gchar *filename;

        filename = g_build_filename (g_get_home_dir (), ".glosung", file, NULL);
        if (access (filename, F_OK | R_OK)) {
                g_free (filename);
                filename = g_strdup_printf (GLOSUNG_DATA_DIR "/%s", file);
                if (access (filename, F_OK | R_OK)) {
                        g_free (filename);
                        return NULL;
                }
        }

        return filename;
} /* check_file */


/*
 * public function that parses the xml file and return required Losung.
 */
static const Losung*
parse (GDateTime *date, gchar *lang, gchar *filename)
{
        xmlSAXHandlerPtr  sax;

        sax = g_new0 (xmlSAXHandler, 1);
        sax->startElement = start_element;
        sax->endElement = end_element;
        sax->characters = character;
        state = STATE_START;
        datum = date;
        day = g_date_time_get_day_of_year (date);
        found = FALSE;
        date_string = g_date_time_format (date, "%Y-%m-%d");
        ww = g_new0 (Losung, 1);
        string   = g_string_sized_new (LINE_LEN);
        location = g_string_sized_new (LINE_LEN);

        xmlSAXParseFile (sax, filename, 0);

        g_free (filename);
        g_free (date_string);
        g_free (sax);

        quote = NULL;
        if (! found) {
                losung_free (ww);
                ww = NULL;
        }

        return ww;
} /* parse */


/*
 * callback function for SAX when an beginning of a tag is found.
 */
static void
start_element (void *ctx, const xmlChar *name, const xmlChar **attrs)
{
        if (depth > 0) {
                depth++;
                return;
        }

        switch (state) {
        case STATE_START:
                if (switch_state (name, STATE_LOSFILE)) {
                } else if (switch_state (name, TW_THE_WORD_FILE)) {
                } else if (switch_state (name, LOS_DATAROOT)) {
                } else if (switch_state (name, LOS_FREE_XML)) {}
                break;
        case STATE_LOSFILE:
        case TW_THE_WORD_FILE:
        case LOS_DATAROOT:
        case LOS_FREE_XML:
                if (switch_state (name, STATE_HEAD)
                  || switch_state (name, STATE_YEAR)
                  || switch_state (name, STATE_MONTH)
                  || switch_state (name, STATE_DAY)
                  || switch_state (name, TW_HEAD))
                {
                        depth = 1;
                } else if (switch_state (name, STATE_LOSUNG)
                  || switch_state (name, LOS_LOSUNGEN))
                {
                        if (--day != 0) {
                                depth = 1;
                        } else {
                                found = TRUE;
                        }
                } else if (switch_state (name, TW_THE_WORD)) {
                        if (found || (strcmp ((const gchar*) attrs [1],
                                              date_string) != 0))
                        {
                                depth = 1;
                        } else {
                                found = TRUE;
                        }
                }
                break;
        case LOS_LOSUNGEN:
        {
                /* title: timestring according to 'man strftime' */
                ww->title = g_date_time_format (datum, _("Watchword for %A, %e. %B %Y"));
        }
                if        (switch_state (name, LOS_DATUM)) {
                } else if (switch_state (name, LOS_WTAG)) {
                } else if (switch_state (name, LOS_SONNTAG)) {
                } else if (switch_state (name, LOS_LOSUNGSTEXT)) {
                        quote = &ww->ot;
                } else if (switch_state (name, LOS_LOSUNGSVERS)) {
                        quote = &ww->ot;
                } else if (switch_state (name, LOS_LEHRTEXT)) {
                        quote = &ww->nt;
                } else if (switch_state (name, LOS_LEHRTEXTVERS)) {
                        quote = &ww->nt;
                }
                break;
        case STATE_LOSUNG:
                if        (switch_state (name, STATE_TL)) {
                } else if (switch_state (name, STATE_OT)) {
                        quote = &ww->ot;
                } else if (switch_state (name, STATE_NT)) {
                        quote = &ww->nt;
                } else if (switch_state (name, STATE_TT)) {
                        depth = 1;
                } else if (switch_state (name, STATE_SR)) {
                } else if (switch_state (name, STATE_CR)) {
                } else if (switch_state (name, STATE_C)) {
                }
                break;
        case TW_THE_WORD:
                if        (switch_state (name, TW_TITLE)) {
                } else if (switch_state (name, TW_PAROL)) {
                        if (! quote) {
                                quote = &ww->ot;
                        } else {
                                quote = &ww->nt;
                        }
                        quote->location_sword = g_strconcat ("sword:///",
                                sword_book_title_for_the_word (attrs [1]),
                                attrs [3], ".", attrs [5], NULL);
                }
                break;
        case TW_PAROL:
                if        (switch_state (name, TW_INTRO)) {
                        g_string_append (string, "<i>");
                } else if (switch_state (name, TW_TEXT)) {
                } else if (switch_state (name, TW_REF)) {
                }
                break;
        case STATE_OT:
        case STATE_NT:
                if        (switch_state (name, STATE_S)) {
                        quote->location_sword = g_strconcat (
                                "sword:///", sword_book_title (attrs [1]),
                                attrs [3], ".", attrs [5], NULL);
                } else if (switch_state (name, STATE_L)) {
                } else if (switch_state (name, STATE_SL)) {
                } else if (switch_state (name, STATE_IL)) {
                        g_string_append (string, "<i>");
                }
                break;
        case STATE_C:
                if (switch_state (name, STATE_L)) {}
                break;
        case STATE_SR:
        case STATE_CR:
                if (switch_state (name, STATE_SL)) {}
                break;
        case TW_TEXT:
        case STATE_L:
                if (switch_state (name, STATE_EM)
                    || switch_state (name, TW_EM))
                {
                        g_string_append (string, "<b>");
                }
                break;
        default:
                g_message ("unknown tag %s.", name);
                g_assert_not_reached ();
                break;
        }
} /* start_element */


/*
 * callback function for SAX when an end of a tag is found.
 */
static void
end_element (void *ctx, const xmlChar *name)
{
        // g_message ("END OF %s %d", name, depth);
        if (depth > 0) {
                depth--;
                if (depth == 0) {
                        pop_state ();
                }
                return;
        }

        switch (state) {
        case STATE_TL:
        case TW_TITLE:
                ww->title = get_string (string);
                break;
        case LOS_LOSUNGSTEXT:
        case LOS_LEHRTEXT:
                quote->text = get_string_with_markup (string);
                break;
        case LOS_LOSUNGSVERS:
        case LOS_LEHRTEXTVERS:
                quote->location = get_string (location);
                quote->location_sword =
                        sword_link_for_original_losung (quote->location);
                break;
        case TW_PAROL:
        case STATE_OT:
        case STATE_NT:
                quote->text = get_string (string);
                quote->location = get_string (location);
                break;
        case STATE_SR:
                ww->selective_reading = get_string (location);
                break;
        case STATE_CR:
                ww->continuing_reading = get_string (location);
                break;
        case STATE_C:
                ww->comment = get_string (string);
                break;
        case STATE_L:
                g_string_append_c (string, '\n');
                break;
        case TW_INTRO:
        case STATE_IL:
                g_string_append (string, "</i>");
                quote->say = get_string (string);
                break;
        case TW_EM:
        case STATE_EM:
                g_string_append (string, "</b>");
                break;
        default:
                break;
        }
        pop_state ();
} /* end_element */


/*
 * callback function for SAX when any character outside of tags are found.
 */
static void
character (void *ctx, const xmlChar *ch, int len)
{
        if (depth > 0) {
                return;
        }

        switch (state) {
        case LOS_LOSUNGSTEXT:
        case LOS_LEHRTEXT:
        case TW_TITLE:
        case TW_INTRO:
        case TW_EM:
        case TW_TEXT:
        case STATE_TL:
        case STATE_IL:
        case STATE_EM:
        case STATE_L:
                g_string_append_len (string, (gchar *) ch, len);
                break;
        case LOS_LOSUNGSVERS:
        case LOS_LEHRTEXTVERS:
        case TW_REF:
        case STATE_SL:
                g_string_append_len (location, (gchar *) ch, len);
                break;
        default:
                break;
        }
} /* character */


static gboolean
switch_state (const xmlChar *name, State newState)
{
        if (strcmp ((char *) name, states [newState]) != 0) {
                return FALSE;
        }
        // g_message ("switch state %s", states [newState]);
        stack = g_slist_prepend (stack, GINT_TO_POINTER (state));
        state = newState;
        return TRUE;
} /* switch_state */


static void
pop_state (void)
{
        if (! stack) {
                g_message ("old state = %s (%d)", states [state], state);
                g_assert_not_reached ();
                return;
        }
        state = GPOINTER_TO_INT (stack->data);
        GSList *list;
        list = stack;
        stack = stack->next;
        g_slist_free_1 (list);
} /* pop_state */


static gchar*
get_string (GString *string)
{
        if (string->str [string->len - 1] == '\r'
            || string->str [string->len - 1] == '\n')
        {
                string->len--;
                string->str [string->len] = '\0';
        }
        string->len = 0;
        return g_strdup (string->str);
} /* get_string */


static gchar*
get_string_with_markup (GString *string)
{
        int index;

        if (string->str [string->len - 1] == '\r'
            || string->str [string->len - 1] == '\n')
        {
                string->len--;
                string->str [string->len] = '\0';
        }
        for (index = 0; index < string->len; index++) {
                if (string->str [index] == '#' || string->str [index] == '/') {
                        break;
                }
        }
        if (index != string->len) {
                gchar   *result;
                gboolean bold    = FALSE;
                gboolean italic  = FALSE;
                GString *string2 = g_string_new_len (string->str, index);

                do {
                switch (string->str [index]) {
                case '#':
                        if (! bold) {
                                g_string_append (string2, "<b>");
                        } else {
                                g_string_append (string2, "</b>");
                        }
                        bold = ! bold;
                        break;
                case '/':
                        if (! italic) {
                                g_string_append (string2, "<i>");
                        } else {
                                g_string_append (string2, "</i>");
                        }
                        italic = ! italic;
                        break;
                }
                for (index++; index < string->len; index++) {
                        if (string->str [index] == '#'
                            || string->str [index] == '/')
                        {
                                break;
                        } else {
                                g_string_append_c
                                        (string2, string->str [index]);
                        }
                }
                } while (index < string->len);

                result = g_strdup (string2->str);
                g_string_free (string2, TRUE);
                string->len = 0;
                return result;
        }

        string->len = 0;
        return g_strdup (string->str);
} /* get_string_with_markup */


/*
 * 0: sword book name
 * 1: old losung book name abbreviation
 * 2: original german Losung book name
 */
static gchar const * const books [][3] = {
        {"Genesis",         "Gn",   "1.Mose"},
        {"Exodus",          "Ex",   "2.Mose"},
        {"Leviticus",       "Lv",   "3.Mose"},
        {"Numbers",         "Nu",   "4.Mose"},
        {"Deuteronomy",     "Dt",   "5.Mose"},
        {"Joshua",          "Jos",  "Josua"},
        {"Judges",          "Jdc",  "Richter"},
        {"Ruth",            "Rth",  "Rut "}, /* add a space for >=4 chars */
        {"I Samuel",        "1Sm",  "1.Samuel"},
        {"II Samuel",       "2Sm",  "2.Samuel"},
        {"I Kings",         "1Rg",  "1.Könige"},
        {"II Kings",        "2Rg",  "2.Könige"},
        {"I Chronicles",    "1Chr", "1.Chronik"},
        {"II Chronicles",   "2Chr", "2.Chronik"},
        {"Ezra",            "Esr",  "Esra"},
        {"Nehemiah",        "Neh",  "Nehemia"},
        {"Esther",          "Esth", "Ester"},
        {"Job",             "Job",  "Hiob"},
        {"Psalms",          "Ps",   "Psalm"},
        {"Proverbs",        "Prv",  "Sprüche"},
        {"Ecclesiastes",    "Eccl", "Prediger"},
        {"Song of Solomon", "Ct",   "Hohelied"},
        {"Isaiah",          "Is",   "Jesaja"},
        {"Jeremiah",        "Jr",   "Jeremia"},
        {"Lamentations",    "Thr",  "Klagelieder"},
        {"Ezekiel",         "Ez",   "Hesekiel"},
        {"Daniel",          "Dn",   "Daniel"},
        {"Hosea",           "Hos",  "Hosea"},
        {"Joel",            "Joel", "Joel"},
        {"Amos",            "Am",   "Amos"},
        {"Obadiah",         "Ob",   "Obadja"},
        {"Jonah",           "Jon",  "Jona"},
        {"Micah",           "Mch",  "Micha"},
        {"Nahum",           "Nah",  "Nahum"},
        {"Habakkuk",        "Hab",  "Habakuk"},
        {"Zephaniah",       "Zph",  "Zefanja"},
        {"Haggai",          "Hgg",  "Haggai"},
        {"Zechariah",       "Zch",  "Sacharja"},
        {"Malachi",         "Ml",   "Maleachi"},
        {"Matthew",         "Mt",   "Matthäus"},
        {"Mark",            "Mc",   "Markus"},
        {"Luke",            "L",    "Lukas"},
        {"John",            "J",    "Johannes"},
        {"Acts",            "Act",  "Apostelgeschichte"},
        {"Romans",          "R",    "Römer"},
        {"I Corinthians",   "1K",   "1.Korinther"},
        {"II Corinthians",  "2K",   "2.Korinther"},
        {"Galatians",       "G",    "Galater"},
        {"Ephesians",       "E",    "Epheser"},
        {"Philippians",     "Ph",   "Philipper"},
        {"Colossians",      "Kol",  "Kolosser"},
        {"I Thessalonians", "1Th",  "1.Thessalonicher"},
        {"II Thessalonians","2Th",  "2.Thessalonicher"},
        {"I Timothy",       "1T",   "1.Timotheus"},
        {"II Timothy",      "2T",   "2.Timotheus"},
        {"Titus",           "Tt",   "Titus"},
        {"Philemon",        "Phm",  "Philemon"},
        {"Hebrews",         "H",    "Hebräer"},
        {"James",           "Jc",   "Jakobus"},
        {"I Peter",         "1P",   "1.Petrus"},
        {"II Peter",        "2P",   "2.Petrus"},
        {"I John",          "1J",   "1.Johannes"},
        {"II John",         "2J",   "2.Johannes"},
        {"III John",        "3J",   "3.Johannes"},
        {"Jude",            "Jd",   "Judas"},
        {"Revelation of John", "Ap", "Offenbarung"},
        {NULL, NULL, NULL}
};


static const gchar*
sword_book_title (const xmlChar* book)
{
        int i = 0;
        while (books [i][0] != NULL) {
                if (strcmp ((char *) books [i][1], (char *) book) == 0) {
                        return books [i][0];
                }
                i++;
        }
        return NULL;
} /* sword_book_title */


static const gchar*
sword_book_title_for_original_losung (const gchar *book)
{
        gchar *book_title;
        int i = 0;
        int len = 4; // strlen (book);

        book_title = (gchar *)book;
        while (*book_title && ! isalpha (*book_title)) {
                book_title++;
        }

        while (books [i][0] != NULL) {
                if (isdigit(books[i][2][0])) {
                        if (strncmp ((char *) books [i][2] + 2, (char *) book_title, len) == 0) {
                                /* base name matches; check for index (one digit only) */
                                if (books [i][2][0] == book [0]) {
                                        return books [i][0];
                                }
                        }
                } else if (strncmp ((char *) books [i][2], (char *) book, len) == 0) {
                        return books [i][0];
                }
                i++;
        }
        return NULL;
} /* sword_book_title_for_original_losung */


static const gchar*
sword_book_title_for_the_word (const xmlChar *book_number)
{
        int i = 0;
        sscanf ((const char *) book_number, "%d", &i);
        return books [i - 1][0];
} /* sword_book_title_for_the_word */


/*
 * construct sword link out of original Losung location string
 */
static gchar*
sword_link_for_original_losung (const gchar *location)
{
        int chapter;
        int verse;
        int index = strlen (location);
        while (index > 0 && location [--index] != ',')
                ;
        sscanf (location + index + 1, "%d", &verse);
        while (index > 0 && location [--index] != ' ')
                ;
        sscanf (location + index + 1, "%d", &chapter);

        return g_strdup_printf
                ("sword:///%s %d.%d",
                 sword_book_title_for_original_losung (location),
                 chapter, verse);
} /* sword_link_for_original_losung */










static gboolean
is_the_word_file (const gchar *name, int len, gchar* lang, gint year)
{
        if ((strncmp (name + len -  4, ".twd", 4)) == 0) {
                gchar *file_lang = g_strndup (name, 2);
                int    file_year = -1;
                if (sscanf (name + len - 8, "%d", &file_year) == 0) {
                        sscanf (name + 3, "%d", &file_year);
                }
                if (file_year == year && strncmp (lang, file_lang, 2) == 0) {
                        return TRUE;
                }
        }
        return FALSE;
} /* is_the_word_file */


static const gchar*
find_the_word_file_in_dir (gchar *dirname, gchar* lang, gint year)
{
        if (access (dirname, F_OK | R_OK) != 0) {
                return NULL;
        }
        GDir  *dir = g_dir_open (dirname, 0, NULL);
        const gchar *name;

        while ((name = g_dir_read_name (dir)) != NULL) {
                int len = strlen (name);
                if (is_the_word_file (name, len, lang, year)) {
                        return name;
                }
        }
        return NULL;
} /* find_the_word_file_in_dir */


static const gchar*
find_the_word_file (gchar *lang, gint year)
{
        gchar *dirname;
        const gchar *filename;

        filename = find_the_word_file_in_dir (GLOSUNG_DATA_DIR, lang, year);
        if (filename != NULL) {
                return filename;
        }
        dirname = g_build_filename (g_get_home_dir (), ".glosung", NULL);
        filename = find_the_word_file_in_dir (dirname, lang, year);

        return filename;
} /* find_the_word_file */
