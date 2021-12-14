Summary: Drakon provides a simple way to manage UNIX processes.
Name: %app_name
Version: %pkg_version
Release: 1
License: GPL
Group: System/Monitoring
Source: %pkg_name
Requires: libX11, libXinerama, libXpm, libXft, libxml2, gd, libjpeg, libpng, libicu, pcre

%description
Drakon provides a simple way to manage UNIX processes.
(http://www.newplanetsoftware.com/drakon/)

%prep
%setup

%install

%define drakon_doc_dir  /usr/share/doc/drakon
%define gnome_app_path  /usr/share/applications
%define gnome_icon_path /usr/share/pixmaps

./install "$RPM_BUILD_ROOT"/usr

%files

%docdir %drakon_doc_dir

/usr/bin/drakon
%drakon_doc_dir

%gnome_app_path/drakon.desktop
%gnome_icon_path/drakon.xpm
