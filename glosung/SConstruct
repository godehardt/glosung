# SConstruct
# Copyright (C) 1999-2009 Eicke Godehardt

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

version = '3.3.2'

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
pixmap_dir  = '/share/pixmaps/'
#pixmap_dir  = ''
data_dir  = prefix + '/share/glosung'
#data_dir  = ''
doc_dir     = '/share/doc/glosung-' + version

BuildDir ('build', 'src')

cpppath = ['#', '#build']
ccflags   = ['-O2', '-Wall', '-g',
#		'-DLIBXML_STATIC',
		'-DVERSION=\\"' + version + '\\"',
		'-DGLOSUNG_DATA_DIR=\\"' + data_dir + '\\"',
		'-DPACKAGE_PIXMAPS_DIR=\\"' + prefix + pixmap_dir + '\\"']

linkflags = ['-Wl,--export-dynamic', '-L.']
#  -L/usr/lib'
#             `pkg-config --libs gtk+-2.0 libxml-2.0 gconf-2.0 libcurl`
             

if ARGUMENTS.get ('profile'):
    ccflags   += ' -pg -fprofile-arcs'
    linkflags += ' -pg -fprofile-arcs'

#if not (ARGUMENTS.get ('dev')):
if (ARGUMENTS.get ('dev')):
    ccflags   += ' -Werror -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED'

tar_file = '#../glosung-' + version + '.tar.bz2'

env = Environment (
  LINK      = 'gcc',
  CC        = 'gcc',
  CPPPATH   = cpppath,
  LINKFLAGS = linkflags,
  CCFLAGS   = ccflags,
  ENV       = os.environ,
  TARFLAGS  = '-c -j')

if env['PLATFORM'] == 'win32':
        env.ParseConfig ('pkg-config gtk+-2.0 libxml-2.0 libcurl --cflags --libs')
        Tool('mingw')(env)
else:
        env.ParseConfig('pkg-config gtk+-2.0 libxml-2.0 gconf-2.0 libcurl --cflags --libs')
#        Tool('posix')(env)


conf = Configure (env)
if not conf.CheckLib ('libxml2'):
	print 'Did not find libxml2.a or xml2.lib, exiting!'

Export ('env cpppath ccflags install_dir pixmap_dir tar_file')

SConscript ('build/SConscript')
SConscript ('po/SConscript')

env.Alias ('install', install_dir)
env.Alias ('package', tar_file)

env.Install (dir = install_dir + doc_dir,
          source = ['AUTHORS', 'COPYING', 'ChangeLog', 'INSTALL', 'README'])

env.Install (dir = install_dir + '/share/applications',
          source = 'glosung.desktop')
env.Install (dir = install_dir + pixmap_dir, source = 'glosung.png')
env.Install (dir = install_dir + pixmap_dir, source = 'glosung-big.png')

# TODO put everything into a folder "glosung-<VERSION>" and rename build to src
if env['PLATFORM'] != 'win32':
	env.Tar (tar_file, ['AUTHORS',
                    'COPYING',
                    'ChangeLog',
                    'INSTALL',
                    'NEWS',
                    'README',
                    'SConstruct',
                    'glosung-big.png',
                    'glosung.desktop',
                    'glosung.png',
                    'debian/glosung.files',
                    'debian/control',
                    'debian/changelog',
                    'debian/copyright',
                    'debian/rules',
                    'rpm/glosung.spec'])

# dpkg-buildpackage -tc -us -uc -rfakeroot
# rpmbuild -ba rpm/glosung.spec
