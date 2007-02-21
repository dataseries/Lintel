#
# SPEC file for Lintel
#
Name:           Lintel
Version:        1.0.0
URL:            http://twiki.hpl.hp.com/bin/view/StorageSystems/TiColi
Source:         %{name}-%{version}.tar.gz
Release:        1
Summary:        Ticoli FUSE logging
Group:          System Environment/Base
License:        Closed
Packager:	Chip Christian <chip.christian@hp.com>
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Requires:	fuse
Requires:	fuse-libs

%description
Ticoli logging package for FUSE.

%package	libs
Summary:        Lintel library files
Group:          Libraries
Requires:       %{name}-libs = %{version}-%{release}
Requires:       pkgconfig
License:        Closed

%package	devel
Summary:        Lintel devel files
Group:          Development/Libraries
Requires:       %{name}-devel = %{version}-%{release}
Requires:       pkgconfig
License:        Closed

%description	libs
Lintel libraries

%description	devel
Lintel header files

%prep
%setup -q

%build
autoreconf --install
mkdir build
cd build
../configure --prefix=/usr --enable-optmode=optimize --enable-maintainer-mode
make

%install
cd build
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/bin/batch-parallel
/usr/bin/buildTcl
/usr/bin/calcStats
/usr/bin/drawRandomLogNormal
/usr/bin/lintel-acinclude
/usr/bin/lintel-config
/usr/bin/mercury-plot
/usr/share/bp_modules/BatchParallel/common.pm
/usr/share/bp_modules/BatchParallel/compress.pm
/usr/share/bp_modules/BatchParallel/jobsfile.pm
/usr/share/perl5/Plot/Mercury.pm
/usr/share/perl5/Text/ExpandInt.pm

%files devel
/usr/include/Lintel/AssertBoost.H
/usr/include/Lintel/AssertException.H
/usr/include/Lintel/Clock.H
/usr/include/Lintel/CompilerMarkup.H
/usr/include/Lintel/Deque.H
/usr/include/Lintel/Double.H
/usr/include/Lintel/HashMap.H
/usr/include/Lintel/HashTable.H
/usr/include/Lintel/HashUnique.H
/usr/include/Lintel/LintelAssert.H
/usr/include/Lintel/LintelVersion.H
/usr/include/Lintel/MathSpecialFunctions.H
/usr/include/Lintel/Matrix.H
/usr/include/Lintel/MersenneTwisterRandom.H
/usr/include/Lintel/PThread.H
/usr/include/Lintel/Posix.H
/usr/include/Lintel/PriorityQueue.H
/usr/include/Lintel/Stats.H
/usr/include/Lintel/StatsEMA.H
/usr/include/Lintel/StatsHistogram.H
/usr/include/Lintel/StatsMaker.H
/usr/include/Lintel/StatsQuantile.H
/usr/include/Lintel/StatsRW.H
/usr/include/Lintel/StatsSequence.H
/usr/include/Lintel/StatsSeries.H
/usr/include/Lintel/StatsSeriesGroup.H
/usr/include/Lintel/StringUtil.H
/usr/include/Lintel/Uncompress.H
/usr/include/Lintel/streamcompat.H
/usr/share/aclocal/com_hp_hpl_lintel.m4
/usr/lib/Make.common
/usr/lib/Make.world
/usr/lib/libLintel.a
/usr/lib/libLintel.la
/usr/lib/libLintel.so.0.0.0
/usr/lib/libLintelPThread.a
/usr/lib/libLintelPThread.la


%files libs
/usr/lib/libLintel.so.0.0.0
/usr/lib/libLintelPThread.so.0.0.0

%changelog

* Wed Feb 14 2007 Chip Christian <chip.christian@hp.com>
- Initial RPM release.
make
