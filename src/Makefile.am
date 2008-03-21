## Process this file with automake to produce Makefile.in
#
#
#  (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

# Automake input file to generate Makefile.in for configure

# avoid requiring all GNUisms
AUTOMAKE_OPTIONS = 1.7 foreign

bin_PROGRAMS = calcStats drawRandomLogNormal 
bin_SCRIPTS = buildTcl batch-parallel mercury-plot lintel-config lintel-acinclude

calcStats_SOURCES = calcStats.cpp
drawRandomLogNormal_SOURCES = drawRandomLogNormal.cpp

LDADD = libLintel.la @TCL_LIBS@ 
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include @TCL_CFLAGS@ @GC_CFLAGS@ -DLIBLINTEL_VERSION="\"@LIBLINTEL_VERSION@\"" 
AM_LDFLAGS = @OPTMODE_LDFLAGS@

lib_LTLIBRARIES = libLintel.la libLintelPThread.la 
if WITH_GC
lib_LTLIBRARIES += libLintelGC.la libnogc.la
endif

libLintel_la_LDFLAGS = -version-info @LIBLINTEL_VERSION@
libLintel_la_LIBADD = @TCL_LIBS@
 
libLintel_la_SOURCES = \
	AssertException.cpp \
	Clock.cpp \
	ConstantString.cpp \
	Deque.cpp \
	Double.cpp \
	HashTable.cpp \
	LeastSquares.cpp \
	LintelAssert.cpp \
	LintelVersion.cpp \
	MathSpecialFunctions.cpp \
	Matrix.cpp \
	MersenneTwisterRandom.cpp \
	PriorityQueue.cpp \
	Stats.cpp \
	StatsEMA.cpp \
	StatsHistogram.cpp \
	StatsMaker.cpp \
	StatsRW.cpp \
	StatsQuantile.cpp \
	StatsSequence.cpp \
	StatsSeries.cpp \
	StatsSeriesGroup.cpp \
	StringUtil.cpp \
	Uncompress.cpp

if ENABLE_STDIO64
libLintel_la_SOURCES += stdio_64.cpp
endif

if WITH_BOOST
libLintel_la_SOURCES += AssertBoost.cpp
endif

if WITH_TCL
BUILT_SOURCES = LintelTclCommands.cpp
libLintel_la_SOURCES += LintelTclCommands.cpp Nameable.cpp \
  Random.cpp Randommwm.cpp TclInterface.cpp 

# Headers for classes which define Tcl-accesible functions:
TCL_HDRS = $(top_srcdir)/include/Lintel/Random.H

# TODO: I think this is a broken way of doing things and that we should
# always rebuild LintelTclCommands.cpp into the build directory

LintelTclCommands.cpp: $(top_srcdir)/include/Lintel/TclInterface.H $(TCL_HDRS) $(top_builddir)/src/buildTcl
	@[ ! -f $(top_srcdir)/src/LintelTclCommands.cpp ] || rm $(top_srcdir)/src/LintelTclCommands.cpp
	$(top_builddir)/src/buildTcl -- -o $@-new -t $(TCL_HDRS)
	@echo "int UseLintelTclCommands = 1; /* here to force linking */" >>$@-new
	mv $@-new $@
endif

if WITH_GC
libLintelGC_la_LDFLAGS = -version-info @LIBLINTELGC_VERSION@ 
libLintelGC_la_LIBADD = libLintel.la
libLintelGC_la_SOURCES = \
	StringId.cpp \
	ConstantString.cpp

libnogc_la_LDFLAGS = -version-info @LIBNOGC_VERSION@
libnogc_la_SOURCES = nogc.c 
endif

if WITH_BOOST
libLintelPThread_la_LDFLAGS = -version-info @LIBLINTELPTHREAD_VERSION@ 
libLintelPThread_la_LIBADD = libLintel.la -lpthread
libLintelPThread_la_SOURCES = \
	PThread.cpp ClockPThread.cpp
endif

if WITH_MAKEBITS
install-exec-local::
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	$(install_sh_DATA) $(srcdir)/Make.common $(DESTDIR)$(libdir)/Make.common
	$(install_sh_DATA) $(srcdir)/Make.world $(DESTDIR)$(libdir)/Make.world
endif

bpmodulesdir = $(datadir)/bp_modules
nobase_bpmodules_DATA = BatchParallel/common.pm BatchParallel/compress.pm BatchParallel/jobsfile.pm BatchParallel/checksum.pm BatchParallel/tar.pm

perllibdir = $(datadir)/perl5
nobase_perllib_DATA = Text/Expand.pm Text/ExpandInt.pm Plot/Mercury.pm Plot/Mercury/Tics.pm Lintel/Net/SSH/KnownHostsFile.pm

packagingdir = $(datadir)/packaging
packaging_DATA = redhat-rules

## TODO: ought to move all this regression stuff to a dejagnu style
## regression test, which is hopefully sufficiently flexible to handle
## the occasionally complex regression tests that we perform.

check_PROGRAMS := tests/check-Stats tests/check-StatsQuantile tests/check-Uncompress tests/check-LintelAssert tests/check-Clock tests/check-hashtable tests/check-mersenneTwister tests/check-least_squares tests/check-rotating_hashmap

if WITH_TCL
check_PROGRAMS += tests/check-Random
endif

check-local: tests/checked_Stats tests/checked_StatsQuantile tests/checked_Uncompress tests/checked_LintelAssert tests/checked_Clock tests/checked_hashtable tests/checked_mersenneTwister tests/checked_least_squares tests/checked_rotating_hashmap
	@echo "Regression tests successful"

clean-local::
	-rm -f tests/checked_*

if WITH_TCL
tests_check_Random_SOURCES = tests/check-Random.cpp

tests/checked_Random: tests/check-Random
	tests/check-Random 2>&1 | diff -b $(srcdir)/@RANDOM_COMPARETO@ -
	touch $@
endif

tests_check_Stats_SOURCES = tests/stats.cpp

tests/checked_Stats: tests/check-Stats
	tests/check-Stats
	touch $@

tests_check_StatsQuantile_SOURCES = tests/stats_quantile.cpp

tests/checked_StatsQuantile: tests/check-StatsQuantile
	tests/check-StatsQuantile | diff -c $(srcdir)/tests/stats_quantile.good -
	touch $@

tests_check_Uncompress_SOURCES = tests/check-Uncompress.cpp

tests/checked_Uncompress: tests/check-Uncompress
	tests/check-Uncompress $(srcdir)/Uncompress.cpp | diff -c $(srcdir)/Uncompress.cpp -
	gzip -1cv <$(srcdir)/Uncompress.cpp >tests/Uncompress.cpp.gz
	tests/check-Uncompress tests/Uncompress.cpp.gz | diff -c $(srcdir)/Uncompress.cpp -
	rm tests/Uncompress.cpp.gz
	touch $@

tests_check_LintelAssert_SOURCES = tests/check-LintelAssert.cpp

tests/checked_LintelAssert: tests/check-LintelAssert $(srcdir)/tests/run-check-LintelAssert.sh
	$(srcdir)/tests/run-check-LintelAssert.sh $(srcdir)
	touch $@

tests_check_Clock_SOURCES = tests/clock.cpp

tests/checked_Clock: tests/check-Clock
	tests/check-Clock
	touch $@

tests_check_hashtable_SOURCES = tests/hashtable.cpp

tests/checked_hashtable: tests/check-hashtable
	tests/check-hashtable
	touch $@

tests_check_mersenneTwister_SOURCES = tests/mersenneTwister.cpp

tests/checked_mersenneTwister: tests/check-mersenneTwister
	tests/check-mersenneTwister
	touch $@

tests_check_least_squares_SOURCES = tests/least_squares.cpp

tests/checked_least_squares: tests/check-least_squares
	./tests/check-least_squares
	touch $@

tests_check_rotating_hashmap_SOURCES = tests/rotating_hashmap.cpp

tests/checked_rotating_hashmap: tests/check-rotating_hashmap
	./tests/check-rotating_hashmap
	touch $@
