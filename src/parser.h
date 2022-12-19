/* glosung.h
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

#ifndef GLOSUNG_PARSER__H
#define GLOSUNG_PARSER__H

#include <glib.h>


typedef struct _Passage Passage;
typedef struct _Losung Losung;

/* represents a scriptural passage */
struct _Passage {
        gchar   *say;           /* e.g. "David said:" */
        gchar   *text;          /* e.g. "Shout to the Lord, all the earth." */
        gchar   *location;      /* e.g. "Psalm 100,1" */
        gchar   *location_sword;/* e.g. "Psalms 100,1" */
};

struct _Losung {
        gchar   *title;         /* e.g. "Losung for Tuesday, August 5, 1975" */
        Passage  ot;            /* passage from old testament */
        Passage  nt;            /* passage from new testament */
        gchar   *comment;       /* e.g. "*** watch for glosung 1.0 ***" */
        gchar   *selective_reading;  /* e.g. "Genesis 2,1-10" */
        gchar   *continuing_reading; /* e.g. "Luke 3,1-6" */
};


void          losung_free     (const Losung *ww);
const Losung *get_orig_losung (GDateTime *date, gchar *lang);
const Losung *get_losung      (GDateTime *date, gchar *lang);
const Losung *get_the_word    (GDateTime *date, gchar *lang);

#endif /* GLOSUNG_PARSER__H */
