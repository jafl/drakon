Summary: Drakon provides a simple way to manage UNIX processes.
Name: %app_name
Version: %pkg_version
Release: 1
License: GPL
Group: System/Monitoring
Source: %pkg_name
Requires: libX11, libXinerama, libXpm, libXft, libxml2, gd, libjpeg, libpng, libicu, pcre, boost-devel, curl

%description
Drakon provides a simple way to manage UNIX processes.

%prep
%setup

%install

%define drakon_doc_dir  /usr/local/share/doc/drakon
%define gnome_app_path  /usr/local/share/applications
%define gnome_icon_path /usr/local/share/pixmaps

./install $RPM_BUILD_ROOT/usr/local

%files

%docdir %drakon_doc_dir

/usr/local/bin/drakon
%drakon_doc_dir

%gnome_app_path/drakon.desktop
%gnome_icon_path/drakon.png
