/* parser.c
 * Copyright (C) 1999-2007 Eicke Godehardt

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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    STATE_LOSUNG,
      STATE_TL,
      STATE_OT,
      STATE_NT,
      STATE_SR,
      STATE_CR,
      STATE_C,
        STATE_IL,
        STATE_S,
        STATE_L,
        STATE_SL,
          STATE_EM,
} State;

static gchar const * const states [] = {
"start", /* dummy string */
  "LOSFILE",
    "HEAD",
    "LOSUNG",
      "TL",
      "OT",
      "NT",
      "SR",
      "CR",
      "C",
        "IL",
        "S",
        "L",
        "SL",
          "EM",
};

static gchar const * const books [][2] = {
        {"Gn",   "Genesis"},
        {"Ex",   "Exodus"},
        {"Lv",   "Leviticus"},
        {"Nu",   "Numbers"},
        {"Dt",   "Deuteronomy"},
        {"Jos",  "Joshua"},
        {"Jdc",  "Judges"},
        {"Rth",  "Ruth"},
        {"1Sm",  "I Samuel"},
        {"2Sm",  "II Samuel"},
        {"1Rg",  "I Kings"},
        {"2Rg",  "II Kings"},
        {"1Chr", "I Chronicles"},
        {"2Chr", "II Chronicles"},
        {"Esr",  "Ezra"},
        {"Neh",  "Nehemiah"},
        {"Esth", "Esther"},
        {"Job",  "Job"},
        {"Ps",   "Psalms"},
        {"Prv",  "Proverbs"},
        {"Eccl", "Ecclesiastes"},
        {"Ct",   "Song of Solomon"},
        {"Is",   "Isaiah"},
        {"Jr",   "Jeremiah"},
        {"Thr",  "Lamentations"},
        {"Ez",   "Ezekiel"},
        {"Dn",   "Daniel"},
        {"Hos",  "Hosea"},
        {"Joel", "Joel"},
        {"Am",   "Amos"},
        {"Ob",   "Obadiah"},
        {"Jon",  "Jonah"},
        {"Mch",  "Micah"},
        {"Nah",  "Nahum"},
        {"Hab",  "Habakkuk"},
        {"Zph",  "Zephaniah"},
        {"Hgg",  "Haggai"},
        {"Zch",  "Zechariah"},
        {"Ml",   "Malachi"},
        {"Mt",   "Matthew"},
        {"Mc",   "Mark"},
        {"L",    "Luke"},
        {"J",    "John"},
        {"Act",  "Acts"},
        {"R",    "Romans"},
        {"1K",   "I Corinthians"},
        {"2K",   "II Corinthians"},
        {"G",    "Galatians"},
        {"E",    "Ephesians"},
        {"Ph",   "Philippians"},
        {"Kol",  "Colossians"},
        {"1Th",  "I Thessalonians"},
        {"2Th",  "II Thessalonians"},
        {"1T",   "I Timothy"},
        {"2T",   "II Timothy"},
        {"Tt",   "Titus"},
        {"Phm",  "Philemon"},
        {"H",    "Hebrews"},
        {"Jc",   "James"},
        {"1P",   "I Peter"},
        {"2P",   "II Peter"},
        {"1J",   "I John"},
        {"2J",   "II John"},
        {"3J",   "III John"},
        {"Jd",   "Jude"},
        {"Ap",   "Revelation of John"},
        {NULL, NULL},
};



/* buf_len for the string which contains the text while parsing */
#define LINE_LEN 80

static State      state;
static GSList    *stack;
static gint       depth;
static gint       day;
static Losung    *ww = NULL;
static Scripture *quote = NULL;
static GString   *string;
static GString   *location;

/****************************\
      Function prototypes
\****************************/

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
static gchar* pop_string     (GString        *string);

static gint sword_book_title (const xmlChar  *book);
static gchar* check_file     (gchar          *directory,
                              guint           year,
                              gchar          *lang,
                              gchar          *prefix);

/*
 * public function that frees the Losung allocated by get_losung.
 */
void
losung_free (Losung *ww)
{
        if (ww->ot.say != NULL) {
                g_free (ww->ot.say);
        }
        g_free (ww->ot.text);
        g_free (ww->ot.location);
   
        if (ww->nt.say != NULL) {
                g_free (ww->nt.say);
        }
        g_free (ww->nt.text);
        g_free (ww->nt.location);

        g_free (ww->title);
        g_free (ww->comment);

        g_free ((Losung *)ww);
} /* losung_free */


/*
 * public function that parses the xml file and return required Losung.
 */
Losung*
get_losung (GDate *date, gchar *lang)
{
        xmlSAXHandlerPtr  sax;
        gchar            *filename;
        guint             year;

        year = g_date_get_year (date) % 100;
        filename = check_file ("/.glosung", year, lang, getenv ("HOME"));
        if (! filename) {
                filename = check_file (GLOSUNG_DATA_DIR, year, lang, "");
        }
        if (! filename) {
                return NULL;
        }

        sax = g_new0 (xmlSAXHandler, 1);
        sax->startElement = start_element;
        sax->endElement = end_element;
        sax->characters = character;
        state = STATE_START;
        day = g_date_get_day_of_year (date);
        ww = g_new0 (Losung, 1);
        string   = g_string_sized_new (LINE_LEN);
        location = g_string_sized_new (LINE_LEN);

        xmlSAXParseFile (sax, filename, 0);

        g_free (filename);
        g_free (sax);
        return ww;
} /* get_losung */


/*
 * This function will check, if the watch word file exists in given
 * directory
 */
static gchar*
check_file (gchar *directory, guint year, gchar *lang, gchar *prefix)
{
        gchar *filename;

        if (year < 10) {
                filename = g_strdup_printf
                        ("%s%s/%s_los0%d.xml", prefix, directory, lang, year);
        } else {
                filename = g_strdup_printf
                        ("%s%s/%s_los%d.xml", prefix, directory, lang, year);
        }

        if (access (filename, F_OK | R_OK) != 0) {
                g_free (filename);
                return NULL;
        }
        return filename;
} /* check_file */


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
                if (switch_state (name, STATE_LOSFILE)) {}
                break;
        case STATE_LOSFILE:
                if (switch_state (name, STATE_HEAD)) {
                        depth = 1;
                } else if (switch_state (name, STATE_LOSUNG)) {
                        if (--day != 0) {
                                depth = 1;
                        }
                }
                break;
        case STATE_LOSUNG:
                if        (switch_state (name, STATE_TL)) {
                } else if (switch_state (name, STATE_OT)) {
                        quote = &ww->ot;
                } else if (switch_state (name, STATE_NT)) {
                        quote = &ww->nt;
                } else if (switch_state (name, STATE_SR)) {
                } else if (switch_state (name, STATE_CR)) {
                } else if (switch_state (name, STATE_C)) {
                }
                break;
        case STATE_OT:
        case STATE_NT:
                if        (switch_state (name, STATE_S)) {
                        gint book = sword_book_title (attrs [1]);
                        quote->location_sword = g_strconcat (
                                "sword:///", books [book][1],
                                attrs [3], ".", attrs [5], NULL);
                        quote->book    = book + 1;
                        sscanf ((const char*) attrs [3], "%d",
                                &(quote->chapter));
                        sscanf ((const char*) attrs [5], "%d",
                                &(quote->verse));
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
        case STATE_L:
                if (switch_state (name, STATE_EM)) {
                        g_string_append (string, "<b>");
                }
                break;
        default:
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
        if (depth > 0) {
                depth--;
                if (depth == 0) {
                        pop_state ();
                }
                return;
        }

        switch (state) {
        case STATE_TL:
                ww->title = pop_string (string);
                break;
        case STATE_OT:
        case STATE_NT:
                string->len--;
                string->str [string->len] = '\0';
                quote->text = pop_string (string);
                quote->location = pop_string (location);
                quote = NULL;
                break;
        case STATE_SR:
                ww->selective_reading = pop_string (location);
                break;
        case STATE_CR:
                ww->continuing_reading = pop_string (location);
                break;
        case STATE_C:
                ww->comment = pop_string (string);
                break;
        case STATE_L:
                g_string_append_c (string, '\n');
                break;
        case STATE_IL:
                g_string_append (string, "</i>");
                quote->say = pop_string (string);
                break;
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
        case STATE_TL:
        case STATE_IL:
        case STATE_EM:
        case STATE_L:
                g_string_append_len (string, (gchar *) ch, len);
                break;
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
        stack = g_slist_prepend (stack, GINT_TO_POINTER (state));
        state = newState;
        return TRUE;
} /* switch_state */


static void
pop_state (void)
{
        state = GPOINTER_TO_INT (stack->data);
        {
                GSList *list;
                list = stack;
                stack = stack->next;
                g_slist_free_1 (list);
        }
} /* pop_state */


static gchar*
pop_string (GString *string)
{
        string->len = 0;
        return g_strdup (string->str);
} /* pop_string */


static gint
sword_book_title (const xmlChar* book)
{
        gint i = 0;
        while (books [i][0] != NULL) {
                if (strcmp ((char *) books [i][0], (char *) book) == 0) {
                        return i;
                }
                i++;
        }
        return -1;
} /* sword_book_title */