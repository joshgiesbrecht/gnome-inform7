# gnome-inform7.spec.  Generated from gnome-inform7.spec.in by configure.
#
# Spec file for GNOME Inform 7
#

Name: gnome-inform7
Version: 5Z71
Release: 1

URL: http://www.inform-fiction.org/
License: GPL

Group: Development/Languages
Source: gnome-inform7-5Z71.tar.gz
# Packager: P. F. Chimento <philip.chimento@gmail.com>
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Requires(pre): GConf2
Requires(post): GConf2
Requires(preun): GConf2

Summary: An IDE for the Inform 7 interactive fiction programming language

%description
GNOME Inform 7 is a port of the Mac OS X and Windows versions of the integrated
development environment for Inform 7. Inform 7 is a "natural" programming
language for writing interactive fiction (also known as text adventures.)

%prep
%setup -q

%build
%configure --enable-manuals
make

%pre
if [ "$1" -gt 1 ] ; then
  GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` \
    gconftool-2 --makefile-uninstall-rule \
    %{_sysconfdir}/gconf/schemas/[NAME] .schemas >/dev/null || :
fi

%install
rm -rf %{buildroot}
export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
%makeinstall
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL

%post
GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` \
  gconftool-2 --makefile-install-rule \
  %{_sysconfdir}/gconf/schemas/%{name}.schemas &>/dev/null || :

%preun
if [ "$1" -eq 0 ] ; then
  GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` \
    gconftool-2 --makefile-uninstall-rule \
    %{_sysconfdir}/gconf/schemas/%{name}.schemas &>/dev/null || :
fi

%clean
rm -rf %{buildroot}

%files
%define pkgdatadir %{_datadir}/%{name}
%define pkgdocdir %{_datadir}/doc/%{name}
%define pkglibexecdir %{_libexecdir}/%{name}
%defattr(-, root, root)
%docdir %{pkgdocdir}
%docdir %{pkgdatadir}/Documentation
%{_datadir}/applications/%{name}.desktop
%{_sysconfdir}/gconf/schemas/%{name}.schemas
%{pkgdocdir}/AUTHORS 
%{pkgdocdir}/ChangeLog 
%{pkgdocdir}/COPYING 
%{pkgdocdir}/NEWS 
%{pkgdocdir}/README 
%{pkgdocdir}/THANKS 
%{pkgdocdir}/TODO
%{pkgdatadir}/uninstall_manifest.txt
%{pkgdatadir}/Documentation/*.html
%{pkgdatadir}/Documentation/*.png
%{pkgdatadir}/Documentation/*.gif
%{pkgdatadir}/Documentation/manifest.txt
%{pkgdatadir}/Documentation/doc_images/*.png
%{pkgdatadir}/Documentation/doc_images/*.jpg
%{pkgdatadir}/Documentation/doc_images/*.tif
%{pkgdatadir}/Documentation/map_icons/*.png
%{pkgdatadir}/Documentation/scene_icons/*.png
%{pkgdatadir}/Documentation/Sections/*.html
%{pkgdatadir}/Extensions/David*Fisher/English.i7x
%{pkgdatadir}/Extensions/Emily*Short/*.i7x
%{pkgdatadir}/Extensions/Graham*Nelson/*.i7x
%{pkgdatadir}/Extensions/Reserved/*.i6t
%{pkgdatadir}/Extensions/Reserved/*.jpg
%{pkgdatadir}/Extensions/Reserved/*.html
%{pkgdatadir}/Extensions/Reserved/IntroductionToIF.pdf
%{pkgdatadir}/Extensions/Reserved/Templates/Classic/*.html
%{pkgdatadir}/Extensions/Reserved/Templates/Standard/*.html
%{pkgdatadir}/Extensions/Reserved/Templates/Standard/style.css
%{pkgdatadir}/languages/*.lang
%{pkgdatadir}/Documentation/licenses/*.html
%{pkgdatadir}/styles/*.xml
%{_datadir}/pixmaps/Inform.png
%{_datadir}/pixmaps/%{name}/*.png
%lang(es) %{_datadir}/locale/es/LC_MESSAGES/%{name}.mo
%{_bindir}/gnome-inform7
%{pkglibexecdir}/cBlorb
%{pkgdocdir}/cBlorb/Complete.pdf
%{pkglibexecdir}/gtkterp-frotz
%{pkgdocdir}/frotz/AUTHORS
%{pkgdocdir}/frotz/COPYING
%{pkgdocdir}/frotz/README
%{pkgdocdir}/frotz/TODO
%config %{_sysconfdir}/gtkterp.ini
%{pkgdocdir}/garglk/*.txt
%{pkgdocdir}/garglk/TODO
%{pkglibexecdir}/gtkterp-git
%{pkgdocdir}/git/README.txt
%{pkglibexecdir}/gtkterp-glulxe
%{pkgdocdir}/glulxe/README
%{pkglibexecdir}/inform-6.31-biplatform
%{pkgdocdir}/inform6/readme.txt
%{pkglibexecdir}/ni

%changelog
* Fri Apr 10 2009 P.F. Chimento <philip.chimento@gmail.com>
- Overhauled build process.
* Mon Feb 23 2009 P.F. Chimento <philip.chimento@gmail.com>
- Added the gtkterp-git binary to the packing list.
* Sat Dec 6 2008 P.F. Chimento <philip.chimento@gmail.com>
- Repackaged to release .1 of Public Beta Build 5U92.
* Sun Sep 12 2008 P.F. Chimento <philip.chimento@gmail.com>
- Added scriptlets for GConf2 schemas processing.
* Fri Sep 12 2008 P.F. Chimento <philip.chimento@gmail.com>
- Updated to Public Beta Build 5U92.
* Sat May 3 2008 P.F. Chimento <philip.chimento@gmail.com>
- Fedora 8 release bumped to 2, replacing outdated Glulx Entry Points.
* Wed Apr 30 2008 P.F. Chimento <philip.chimento@gmail.com>
- Updated to Public Beta Build 5T18.
* Mon Dec 3 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to Public Beta Build 5J39.
* Tue Nov 13 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to Public Beta Build 5G67.
* Sat Aug 18 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to version 0.4.
* Sat Jun 16 2007 P.F. Chimento <philip.chimento@gmail.com>
- Repackaged for Fedora 7.
* Sat Jun 2 2007 P.F. Chimento <philip.chimento@gmail.com>
- Repackaged to release 2.
* Sun May 27 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to version 0.3.
* Mon Apr 9 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to version 0.2.
