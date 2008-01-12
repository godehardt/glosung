%define name glosung
%define version 3.2.1
%define release 1

Summary:        Gnome Version of watch words (german: Losung)
Name:           %{name}
Version:        %{version}
Release:        %{release}
Source0:        %{name}-%{version}.tar.bz2
Group:          Applications/Productivity
Vendor:         Eicke Godehardt <eicke AT godehardt DOT org>
Packager:       Sebastian Pätzold <sebastian AT paetzold-family DOT de>
URL:            http://www.godehardt.org/losung.html
BuildRoot:      %{_tmppath}/%{name}-buildroot
Requires:       gtk2 >= 2.10
Requires:       libxml2 >= 2.0
# Requires:       libcurl
# BuildRequires:  libxml-devel >= 2.0
# BuildRequires:  libgtk2-devel >= 2.4
Prefix:         %{_prefix}
# BuildArchitectures: i386
# Distribution:
Copyright:      (C) 1999-2008 Eicke Godehardt
License:        GPL
# This Software is copyright (c) 1999-2008 by Eicke Godehardt.
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
/usr/share/pixmaps/glosung.png
/usr/share/pixmaps/glosung-big.png
/usr/share/locale/cs/LC_MESSAGES/glosung.mo
/usr/share/locale/de/LC_MESSAGES/glosung.mo
/usr/share/locale/fr/LC_MESSAGES/glosung.mo

#%doc README AUTHORS COPYING ChangeLog NEWS TODO
#%doc /usr/share/doc/glosung/README /usr/share/doc/glosung/AUTHORS
#%doc /usr/share/doc/glosung/COPYING /usr/share/doc/glosung/ChangeLog

/usr/share/doc/glosung-%{version}/AUTHORS
/usr/share/doc/glosung-%{version}/COPYING
/usr/share/doc/glosung-%{version}/ChangeLog
/usr/share/doc/glosung-%{version}/INSTALL
/usr/share/doc/glosung-%{version}/README


%changelog
* Mon Dec 31 2007 Eicke Godehardt <eicke@godehardt.org>
- add support for the "original" Losung file format.
- version bump to 3.2.1

* Fri Dec 28 2007 Eicke Godehardt <eicke@godehardt.org>
- show "The Word" automaticaly after 2007
- add a little pad arround all text
- version bump to 3.2

* Sun Sep 30 2007 Eicke Godehardt <eicke@godehardt.org>
- small fix for Gnomesword link
- start only once parameter (--once)
- czech translation update
- version bump to 3.1.1

* Thu Aug 16 2007 Eicke Godehardt <eicke@godehardt.org>
- add keyboard shortcuts
- get rid of gnome libraries
- other small enhancements
- make connection to gnomesword more stable
- Version bump to 3.1

* Sun Feb 11 2007 Eicke Godehardt <eicke@godehardt.org>
- add better language handling for first time users
- add command line arguments to losung
- bump to 3.0.2

* Tue Jan 23 2007 Eicke Godehardt <eicke@godehardt.org>
- fix the crash-at-very-first-startup
- fix handle years for download
- bump to 3.0.1

* Sat Dec 02 2006 Eicke Godehardt <eicke@godehardt.org>
- add libcurl for downloading language files from www.losung.de
- bump to 3.0

* Fri Dec 09 2005 Eicke Godehardt <eicke@godehardt.org>
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
