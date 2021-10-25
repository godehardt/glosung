%define name glosung
%define version 3.6.6
%define release 1

Summary:        Gtk+ Version of watch words (german: Losung)
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source0:        %{name}-%{version}.tar.bz2
Group:          Applications/Productivity
Vendor:         Eicke Godehardt <sf AT godehardt DOT org>
Packager:       Sebastian Pätzold <sebastian AT paetzold-family DOT de>
URL:            http://www.godehardt.org/losung.html
BuildRoot:      %{_tmppath}/%{name}-buildroot
Requires:       gtk2 >= 2.10
Requires:       libxml2 >= 2.0
Requires:       unzip, xdg-utils
# Requires:       libcurl
# BuildRequires:  libxml-devel >= 2.0
# BuildRequires:  libgtk2-devel >= 2.4
Prefix:         %{_prefix}
# BuildArchitectures: i386
# Distribution:
Copyright:      (C) 1999-2021 Eicke Godehardt
License:        GPL
# This Software is copyright (c) 1999-2021 by Eicke Godehardt.
# You are free to distribute this software under the terms
# of the GNU General Public License.

%description
This program shows the watch words (german: Losungen) for each day.
The `Losungen' are words out of the bible, one from the Old
and one from the New Testament.

%prep
%setup

%build
scons
strip build/glosung
strip build/losung

%install
rm -rf $RPM_BUILD_ROOT
scons install DESTDIR=$RPM_BUILD_ROOT
#mkdir -p $RPM_BUILD_ROOT/usr/man/man1
#gzip -c glosung.1 > $RPM_BUILD_ROOT/usr/man/man1/glosung.1.gz

%clean
rm -rf $RPM_BUILD_ROOT $RPM_BUILD_DIR/%{name}-%{version}

%files
%defattr(-,root,root)
/usr/bin/glosung
/usr/bin/losung
/usr/share/applications/glosung.desktop
/usr/share/glosung/glosung.png
/usr/share/glosung/glosung-big.png
/usr/share/glosung/herrnhut.png
/usr/share/glosung/add_language.glade
/usr/share/glosung/preferences.glade
/usr/share/glosung/warning_dialog.glade
/usr/share/locale/cs/LC_MESSAGES/glosung.mo
/usr/share/locale/de/LC_MESSAGES/glosung.mo
/usr/share/locale/fr/LC_MESSAGES/glosung.mo

#%doc README AUTHORS COPYING ChangeLog TODO
#%doc /usr/share/doc/glosung/README /usr/share/doc/glosung/AUTHORS
#%doc /usr/share/doc/glosung/COPYING /usr/share/doc/glosung/ChangeLog

/usr/share/doc/glosung-%{version}/AUTHORS
/usr/share/doc/glosung-%{version}/COPYING
/usr/share/doc/glosung-%{version}/ChangeLog
/usr/share/doc/glosung-%{version}/INSTALL
/usr/share/doc/glosung-%{version}/README


%changelog
*2010-11-01 Eicke
- fix error reporting problem after download (proxy)
- fix translation problem

*2010-10-27 Eicke
- fix XML handling, as file format was slightly by losungen.de
- add [control] modifier to scroll wheel to navigate monthly

*2010-08-04 Eicke
- add Hungarian translation (thanks Mate)
- use scroll wheel to browse through dates
- add proxy support (no authentication so far)
- add option for auto start once a day or at every login

* 2010-05-03  Eicke
- release version 3.5.3
- fix compilation problem under lucid/new gtk versions

* 2010-02-13  Eicke
- release version 3.5.2
- [r267] update debian changelog
- [r266] fix #2947728 and #2935243 (open xiphos and urls in about)

* 2010-01-22  Eicke
- release version 3.5.1
- [r264] fix access key for about menu items
- [r263] fix crash in download manager when selecting bible20
- [r262] update German and Czech translation

* Mon Jan 04 2010 Eicke
- version bump to 3.5
- add functionality to download bible20 texts

* Tue May 26 2009 Eicke
- version bump to 3.4.2
- fix opening of preferences

* Sat May 07 2009 Eicke
- version bump to 3.4.1
- handle new name of GnomeSword, now Xiphos
- further simplify preferences
- some glade than UI code

* Sat Jan 31 2009 Eicke
- version bump to 3.4
- add about Herrnhut
- simplify preferences

* Fri Dec 05 2008 Eicke
- version bump to 3.3.2
- fix path problem finally (hope so)

* Fri Dec 05 2008 Eicke
- version bump to 3.3.1
- fix packaging and path problem

* Wed Dec 03 2008 Eicke
- version bump to 3.3
- add win32 fixes
- clean up
- enhance error handling and return type for autostart

* Mon Nov 24 2008 Eicke
- update German translation
- enhance download, fix UI for download and preferences
- fix windows/linux conflict in SConstruct
- update TODO
- enhance build dependencies for debian control

* Sat Nov 08 2008 Eicke
- make glosung compilable under Windows (mingw)
- remove absolute paths for gettext commands in po/SConscript
- clean up code

* Sat Nov 01 2008 Eicke
- use g_file_test in autostart test
- fix debian files due to upload and build problems on ppa

* Fri Oct 24 2008 Eicke
- add fix for SuSE package size (thanks Sebastian)

* Tue Sep 30 2008 Eicke
- original German xml losungen file can now be downloaded
  and extracted
- set config_path only once

* Sat Sep 27 2008 Eicke
- clean up, especially in download

* Sat Aug 30 2008 Eicke
- finish autostart handling
- finish autostart handling
- enhance desktop file
- add checking for autostart
- correct header

* Wed Aug 27 2008 Eicke
- add initial files for handling autostart settings

* Mon Aug 25 2008 Eicke
- prepare addition of download of original losungen file

* Tue Feb 12 2008 Eicke
- fix command line tool to support same text files as gui
  version
- fix encode setting of czech po file (thanks Marek)
- handle incomplete text files

* Sat Feb 09 2008 Eicke
- hopefully the parser for 'The Word' is fixed now for
  incomplete files
- first changes towards better support of 'The Word' in
  different languages
- add comments
- convert glosung logo to svg
- handle czech los file structure

* Sat Jan 19 2008 Eicke
- add about.c as a source for gettext strings
- try to fix compile warning on openSUSE10.3
- add about.c as a source for gettext strings
- clean up
- do not load originial for other lang than 'de'

* Tue Jan 15 2008 Eicke
- delete losung license texts as no text files are delivered
  with glosung
- fix typo
- add changelog and version bump to spec file

* Sat Jan 12 2008 Eicke
- make file handling more robust
- add important warning message on organize watchword dialog
- fix i18n of menu
- version bump to 3.2.2

* Thu Jan 03 2008 Eicke
- fix text wrapping problem

* Mon Dec 31 2007 Eicke
- add support for the "original" Losung file format.
- version bump to 3.2.1

* Fri Dec 28 2007 Eicke
- show "The Word" automaticaly after 2007
- add a little pad arround all text
- version bump to 3.2

* Sun Sep 30 2007 Eicke
- small fix for Gnomesword link
- start only once parameter (--once)
- czech translation update
- version bump to 3.1.1

* Thu Aug 16 2007 Eicke
- add keyboard shortcuts
- get rid of gnome libraries
- other small enhancements
- make connection to gnomesword more stable
- Version bump to 3.1

* Sun Feb 11 2007 Eicke
- add better language handling for first time users
- add command line arguments to losung
- bump to 3.0.2

* Tue Jan 23 2007 Eicke
- fix the crash-at-very-first-startup
- fix handle years for download
- bump to 3.0.1

* Sat Dec 02 2006 Eicke
- add libcurl for downloading language files from www.losung.de
- bump to 3.0

* Fri Dec 09 2005 Eicke
- move sword flag from compile flag into runtime changable property
- fixed debian package issue
- add texts for 2006
- add swahili localization
- bump to 2.1.3

* Sat Feb 04 2005 Eicke Godehardt <eicke@godehardt-online.de>
- support french for texts and GUI
- fixed compilation bug in SConstruct when PKG_CONFIG_PATH is not defined
- bump to 2.1.2

* Thu Jan 04 2005 Eicke Godehardt <eicke@godehardt-online.de>
- up to date texts for chinese
- bump to 2.1.1

* Thu Dec 23 2004 Eicke Godehardt <eicke@godehardt-online.de>
- texts for 2005
- make ready for gtk+-2.4
- bump to 2.1.0

* Sat Jan 10 2004 Eicke Godehardt <eicke@godehardt-online.de>
- add languages: es, zh-CN, zh-TW
- bump to 2.0.4

* Mon Jan 05 2004 Eicke Godehardt <eicke@godehardt-online.de>
- small bugfixes

* Mon Dec 22 2003 Eicke Godehardt <eicke@godehardt-online.de>
- 2004 update

* Wed Oct  8 2003 Eicke Godehardt <eicke.godehardt@igd.fhg.de>
- small fixes

* Thu Apr  7 2002 Matej Cepl <cepl.m@neu.edu>
- initial RPM
