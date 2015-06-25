Summary: mod_simpleamd for FreeSWITCH
Name: freeswitch-application-simpleamd
Version: 1.4.19
Release: 1%{?dist}
License: MPL 1.1
Group: System Environment/Libraries
URL: https://github.com/crienzo/mod_simpleamd
Source: mod_simpleamd.tar.gz
Requires: freeswitch = %{version}
BuildRequires: pkgconfig autoconf automake libtool freeswitch-devel = %{version} simpleamd-devel

%description
The mod_simpleamd package contains a module for FreeSWITCH to provide answering machine detection

%prep
%setup -c

%build
./bootstrap.sh
PKG_CONFIG_PATH=%{_datadir}/freeswitch/pkgconfig bash -c './configure'

make

%install
make DESTDIR=$RPM_BUILD_ROOT install
rm -f $RPM_BUILD_ROOT%{_libdir}/freeswitch/mod/*.la

%files
%{_libdir}/freeswitch/mod/mod_simpleamd.so

%changelog
* Mon Jun 25 2015 Chris Rienzo <chris@rienzo.com> 1.4.19-1
- Initial revision

