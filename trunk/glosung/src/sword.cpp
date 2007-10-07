/* sword.cpp
 * Copyright (C) 2007 Eicke Godehardt

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
#include <encfiltmgr.h>
#include <swmodule.h>
#include <swmgr.h>
#include <versekey.h>
#include <regex.h>
#include <string.h>
#include <iostream>
#include <glib/gconvert.h>

#include "sword.h"


using sword::SWMgr;
using sword::SWModule;
using sword::EncodingFilterMgr;
using sword::ModMap;
using sword::SectionMap;
using sword::SWKey;
using sword::VerseKey;
using sword::ListKey;
using std::string;


static SWMgr *swMgr = new SWMgr (new EncodingFilterMgr (sword::ENC_UTF8));


gchar *
get_sword_text (gchar *name, int book, int chapter, int verse)
{
	ModMap::iterator it;
	SectionMap::iterator sit;
	SWModule *mod = NULL;

	if (swMgr->Modules.empty () == TRUE) {
                /* FIXME show popup message */
		g_message ("nooooo modules found!");
		return NULL;
	}

	for (it = swMgr->Modules.begin (); it != swMgr->Modules.end (); it++) {
                SWModule *curMod = (*it).second;
		if (! strcmp (curMod->Type (), "Biblical Texts")) {
                        if (! strcmp (curMod->Name (), name)) {
                                mod = curMod;
                                break;
                        }
		}
	}
        if (! mod) {
                return NULL;
        }
        /*
        g_message ("found: %s - %s\n%s - %s",
                   mod->Name (), mod->Description (),
                   mod->Type (), mod->Lang ());
        */

        // (const char *)*curMod;  // snap to closest locations

	VerseKey *key = (VerseKey *)mod->getKey ();
	key->Book   (book);
	key->Chapter (chapter);
	key->Verse  (verse);
	key->Normalize ();
        mod->setKey (key);
        // g_message ("key:  %s", mod->KeyText ());
        gchar *text = (gchar *)mod->StripText ();
        // g_message ("text: %s", text);

        return text;
}


GPtrArray *
get_bibles (void)
{
	ModMap::iterator it;
        GPtrArray *bibles = g_ptr_array_new ();

	for (it = swMgr->Modules.begin (); it != swMgr->Modules.end (); it++) {
                SWModule *curMod = (*it).second;
		if (! strcmp (curMod->Type (), "Biblical Texts")) {
                        g_ptr_array_add (bibles, curMod->Name ());
		}
	}
        return bibles;
}
