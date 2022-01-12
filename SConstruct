# SConstruct
# Copyright (C) 1999-2021 Eicke Godehardt

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


import os

version = '4.0.0a'

# Stores signatures in ".sconsign.dbm"
# in the top-level SConstruct directory.
SConsignFile ()

Default ('.')

Help ('''Options:
  PREFIX=/usr                 Where to install glosung
  DESTDIR=/                   Real installation path DESTDIR + PREFIX
                              (this option is only for packaging)''')

env = Environment (
  LINK      = 'gcc',
  CC        = 'gcc',
  CPPPATH   = '',
  LINKFLAGS = '',
  CCFLAGS   = '',
  ENV       = os.environ,
  TARFLAGS  = '-c -j',
  tools     = ["default", "gettext"])

if env['PLATFORM'] == 'win32':
	prefix      = ARGUMENTS.get ('PREFIX', '')
	install_dir = ARGUMENTS.get ('DESTDIR', '')
	pixmap_dir  = ''
	data_dir    = '.'
	doc_dir     = ''
else:
	prefix      = ARGUMENTS.get ('PREFIX', '/usr')
	install_dir = ARGUMENTS.get ('DESTDIR', '')
	pixmap_dir  = prefix + '/share/glosung/'
	data_dir    = prefix + '/share/glosung'
	doc_dir     = '/share/doc/glosung-' + version


VariantDir('build', 'src')

cpppath = ['#', '#build']
ccflags   = ['-O2', '-std=c99', '-Wall', '-g', '-Wl,—export-dynamic',
#		'-DLIBXML_STATIC',
		'-DVERSION=\\"' + version + '\\"',
		'-DGLOSUNG_DATA_DIR=\\"' + data_dir + '\\"',
		'-DPACKAGE_PIXMAPS_DIR=\\"' + pixmap_dir + '\\"']

linkflags = []          # '-L.'

if ARGUMENTS.get ('profile'):
    ccflags.append   ('-pg', '-fprofile-arcs')
    linkflags.append ('-pg', '-fprofile-arcs')

#if env['PLATFORM'] != 'win32':
#    linkflags.append ('-Wl,--as-needed')

#if not (ARGUMENTS.get ('dev')):
if (ARGUMENTS.get ('dev')):
    ccflags   += [
#         '-Werror',
#         '-pedantic',
    	 '-DG_DISABLE_DEPRECATED', '-DGDK_PIXBUF_DISABLE_DEPRECATED',
     	 '-DGDK_DISABLE_DEPRECATED', '-DGTK_DISABLE_DEPRECATED',
     	 '-DGTK_DISABLE_SINGLE_INCLUDES', '-DGSEAL_ENABLE']
else:
    ccflags   += [
		'-DG_DISABLE_ASSERT']

tar_file = '#../glosung-' + version + '.tar.bz2'

env.Append (
  CPPPATH   = cpppath,
  LINKFLAGS = linkflags,
  CCFLAGS   = ccflags)

if env['PLATFORM'] == 'win32':
        env.ParseConfig ('pkg-config gtk4 libxml-2.0 libcurl --cflags --libs')
        Tool('mingw')(env)
else:
        env.ParseConfig('pkg-config gtk4 libxml-2.0 gconf-2.0 gmodule-export-2.0 libcurl --cflags --libs')
#        Tool('posix')(env)

linkflags.append ('-lgmodule-export-2.0')

conf = Configure (env)
#if not conf.CheckLib ('libxml2'):
#	print('Did not find libxml2.a or xml2.lib, exiting!')

Export ('env cpppath ccflags install_dir prefix pixmap_dir tar_file version')

SConscript ('build/SConscript')
SConscript ('po/SConscript')

env.Alias ('install', install_dir)
env.Alias ('package', tar_file)

env.Install (dir = install_dir + prefix + doc_dir,
          source = ['AUTHORS', 'COPYING', 'ChangeLog', 'INSTALL', 'README'])

env.Install (dir = install_dir + prefix + '/share/applications',
          source = 'glosung.desktop')
env.Install (dir = install_dir + pixmap_dir,
        source = [
                'herrnhut.png',
                'glosung.png',
                'glosung-big.png'])
env.Install (dir = install_dir + data_dir,
        source = [])

# TODO put everything into a folder "glosung-<VERSION>" and rename build to src
if env['PLATFORM'] != 'win32':
	env.Tar (tar_file, ['AUTHORS',
                    'COPYING',
                    'ChangeLog',
                    'INSTALL',
                    'README',
                    'SConstruct',
                    'herrnhut.png',
                    'glosung-big.png',
                    'glosung.desktop',
                    'glosung.png',
                    'glosung.gresource.xml',
                    'ui/add_language.ui',
                    'ui/glosung.ui',
                    'ui/language_manager.ui',
                    'ui/preferences.ui',
                    'ui/warning_dialog.ui',
                    'ui/style.css',
                    'ui/icons/angle-left-solid.png',
                    'ui/icons/angle-right-solid.png',
                    'ui/icons/angles-down-solid.png',
                    'ui/icons/angles-left-solid.png',
                    'ui/icons/bars-solid.png',
                    'ui/icons/calendar-alt-regular.png',
                    'ui/icons/gear-solid.png',
                    'ui/icons/question-circle-regular.png',
                    'debian/glosung.files',
                    'debian/control',
                    'debian/changelog',
                    'debian/copyright',
                    'debian/compat',
                    'debian/rules',
                    'rpm/glosung.spec'])

# dpkg-buildpackage -tc -us -uc -rfakeroot
# rpmbuild -ba rpm/glosung.spec
