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


gchar *
get_sword_text (gchar *name, int book, int chapter, int verse) {
	SWMgr *swMgr = new SWMgr(new EncodingFilterMgr (sword::ENC_UTF8));
	ModMap::iterator it;
	SectionMap::iterator sit;
	SWModule *mod = NULL;

        /* First, check and see whether SWMgr found any installed
         * books.  If not, tell the user what the problem is.  Print a
         * short notice to stderr, for use when the program is invoked
         * from the command line, and also put a bit longer message
         * into the spot where bible text would otherwise show up.
         * Then, return out of this function, because there's nothing
         * left to do.  */
	if (swMgr->Modules.empty () == TRUE) {
		g_message ("nooooo modules found!");
		return NULL;
	}

        /* Next, if SWMgr did find some book modules installed, hook
         * them up to any filters needed and set up the menus to
         * reflect them.  Pick a default module to show on startup.
         */
	for (it = swMgr->Modules.begin (); it != swMgr->Modules.end (); it++) {
                SWModule *curMod = (*it).second;
		if (! strcmp (curMod->Type (), "Biblical Texts")) {
                        // g_message ("module name: %s", curMod->Name ());
                        if (! strcmp (curMod->Name (), name)) {
                                mod = curMod;
                        }
		}
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
