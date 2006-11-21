# SConstruct
# Copyright (C) 1999-2006 Eicke Godehardt

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston,
# MA 02111-1307, USA.

import os

version = '3.0\ Beta'

# Stores signatures in ".sconsign.dbm"
# in the top-level SConstruct directory.
SConsignFile ()

Default ('.')

Help ('''Options:
  PREFIX=/usr                 Where to install glosung
  DESTDIR=/                   Real installation path DESTDIR + PREFIX
                              (this option is only for packaging)''')

prefix      = ARGUMENTS.get ('PREFIX', '/usr')
install_dir = ARGUMENTS.get ('DESTDIR', '') + prefix
pixmap_dir  = '/share/pixmaps'
doc_dir     = '/share/doc/glosung-' + version

BuildDir ('build', 'src')

cpppath = ['#', '#build']
ccflags   = '-Wall -O2 -g -DGLOSUNG_DATA_DIR=\\"' + prefix + '/share/glosung\\" \
            `pkg-config --cflags libgnome-2.0 libgnomeui-2.0 gconf-2.0 libcurl` \
            -DVERSION=\\"' + version + '\\"  \
            -DPACKAGE_PIXMAPS_DIR=\\"' + prefix + pixmap_dir + '\\"'
linkflags = '-Wl,--export-dynamic  -L/usr/lib \
             `pkg-config --libs libgnome-2.0 libgnomeui-2.0 gconf-2.0 libcurl`'

if ARGUMENTS.get ('profile'):
    ccflags   += ' -pg -fprofile-arcs'
    linkflags += ' -pg -fprofile-arcs'

#if not (ARGUMENTS.get ('dev')):
if (ARGUMENTS.get ('dev')):
    ccflags   += ' -Werror -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED'

env = Environment (
  platform  = 'posix',
  LINK      = 'gcc',
  CC        = 'gcc',
  CPPPATH   = cpppath,
  LINKFLAGS = linkflags,
  CCFLAGS   = ccflags,
  ENV       = os.environ)

Export ('env cpppath ccflags install_dir pixmap_dir')

SConscript ('build/SConscript')
SConscript ('po/SConscript')

env.Alias ('install', install_dir)

env.Install (dir = install_dir + doc_dir,
          source = ['AUTHORS', 'COPYING', 'ChangeLog', 'INSTALL', 'README'])
env.Install (dir = install_dir + doc_dir,
          source = ['ENhhut.txt', 'ENhist.txt', 'ENintro.txt', 'ENlicens.txt'])

env.Install (dir = install_dir + '/share/applications',
          source = 'glosung.desktop')
env.Install (dir = install_dir + pixmap_dir, source = 'glosung.png')
env.Install (dir = install_dir + pixmap_dir, source = 'glosung-big.png')

# dpkg-buildpackage -tc -us -uc -rfakeroot
# rpmbuild -ba rpm/glosung.spec
