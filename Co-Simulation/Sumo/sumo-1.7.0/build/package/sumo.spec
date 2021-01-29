#
# spec file for package sumo
#
# Copyright (c) 2018 SUSE LINUX GmbH, Nuernberg, Germany.
# Copyright (c) 2001-2019 DLR (http://www.dlr.de/) and contributors
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


Name:           sumo
Version:        git
Release:        0
# The Epoch line makes sure the nightly build is always considered newer than a release build.
# It should be removed for release builds.
Epoch:          2
Summary:        Eclipse Simulation of Urban Mobility - A Microscopic Traffic Simulation
License:        EPL-2.0
Group:          Productivity/Scientific/Other
URL:            https://sumo.dlr.de/
Source0:        https://sumo.dlr.de/daily/sumo-all-%{version}.tar.gz
BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  python
%if 0%{?suse_version}
BuildRequires:  python-xml
%endif
BuildRequires:  help2man
BuildRequires:  pkgconfig
BuildRequires:  unzip
BuildRequires:  pkgconfig(fox)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xerces-c)
%if 0%{?fedora_version} || 0%{?suse_version}
BuildRequires:  fdupes
BuildRequires:  pkgconfig(gdal)
%endif
%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?scientificlinux_version} || 0%{?suse_version}
BuildRequires:  pkgconfig(proj)
%endif
%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?scientificlinux_version}
BuildRequires:  libGLU-devel
BuildRequires:  libjpeg-devel
BuildRequires:  libpng-devel
BuildRequires:  pkgconfig(libtiff-4)
BuildRequires:  pkgconfig(xcursor)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xft)
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(xrandr)
%endif

%description
"Simulation of Urban MObility" (SUMO) is an open source,
highly portable, microscopic traffic simulation package
designed to handle large road networks.

%package -n libsumocpp
Summary:        Eclipse SUMO - Microscopic Traffic Simulation Library
Group:          Development/Libraries/C and C++

%description -n libsumocpp
libsumocpp provides the C++-API for adding traffic simulation
functionality to your own application.

%if 0%{?fedora_version}
%global debug_package %{nil}
%endif

%prep
%setup -q
# Use real shebang
%if 0%{?fedora_version} > 28
find . -name "*.py" -o -name "*.pyw" | xargs sed -i 's,^#!%{_bindir}/env python$,#!%{_bindir}/python2,'
%else
find . -name "*.py" -o -name "*.pyw" | xargs sed -i 's,^#!%{_bindir}/env python$,#!%{_bindir}/python,'
%endif

%build
mkdir cmake-build
cd cmake-build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make %{?_smp_mflags}
make %{?_smp_mflags} man

%install
cd cmake-build
%make_install
cd ..
ln -s %{_datadir}/sumo/tools/assign/duaIterate.py %{buildroot}%{_bindir}/duaIterate.py
ln -s %{_datadir}/sumo/tools/osmWebWizard.py %{buildroot}%{_bindir}/osmWebWizard.py
ln -s %{_datadir}/sumo/tools/randomTrips.py %{buildroot}%{_bindir}/randomTrips.py
ln -s %{_datadir}/sumo/tools/traceExporter.py %{buildroot}%{_bindir}/traceExporter.py
install -d -m 755 %{buildroot}%{_mandir}/man1
install -p -m 644 docs/man/*.1 %{buildroot}%{_mandir}/man1
install -d -m 755 %{buildroot}%{_sysconfdir}/profile.d
install -p -m 644 build/package/*sh %{buildroot}%{_sysconfdir}/profile.d
install -d -m 755 %{buildroot}%{_datadir}/applications
install -p -m 644 build/package/%{name}.desktop %{buildroot}%{_datadir}/applications
install -d -m 755 %{buildroot}%{_datadir}/pixmaps
install -p -m 644 build/package/%{name}.png %{buildroot}%{_datadir}/pixmaps
%if 0%{?suse_version}
install -d -m 755 %{buildroot}%{_datadir}/mime/application
install -p -m 644 build/package/%{name}.xml %{buildroot}%{_datadir}/mime/application/%{name}.xml
%fdupes -s docs
%fdupes %{buildroot}
%endif

#%check
#cd cmake-build
#make %{?_smp_mflags} test

%post -n libsumocpp -p /sbin/ldconfig
%postun -n libsumocpp -p /sbin/ldconfig

%files
%defattr(-,root,root)
%{_bindir}/*
%{_datadir}/sumo
%doc AUTHORS README.md ChangeLog CONTRIBUTING.md NOTICE.md docs/pydoc docs/userdoc docs/examples docs/tutorial
%if 0%{?suse_version} < 1500
%doc LICENSE
%else
%license LICENSE
%endif
%{_mandir}/man1/*
%config %{_sysconfdir}/profile.d/%{name}.*sh
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/%{name}.png
%if 0%{?suse_version}
%{_datadir}/mime/application
%endif

%files -n libsumocpp
%if 0%{?suse_version} < 1500
%doc LICENSE
%else
%license LICENSE
%endif
%{_libdir}/libsumocpp.so

%changelog
