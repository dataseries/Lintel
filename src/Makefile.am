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

calcStats_SOURCES = calcStats.C
drawRandomLogNormal_SOURCES = drawRandomLogNormal.C

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
	AssertException.C \
	Clock.C \
	ConstantString.C \
	Deque.C \
	Double.C \
	HashTable.C \
	LintelAssert.C \
	LintelVersion.C \
	MathSpecialFunctions.C \
	Matrix.C \
	MersenneTwisterRandom.C \
	PriorityQueue.C \
	Stats.C \
	StatsEMA.C \
	StatsHistogram.C \
	StatsMaker.C \
	StatsRW.C \
	StatsQuantile.C \
	StatsSequence.C \
	StatsSeries.C \
	StatsSeriesGroup.C \
	StringUtil.C \
	Uncompress.C

if ENABLE_STDIO64
libLintel_la_SOURCES += stdio_64.C
endif

if WITH_BOOST
libLintel_la_SOURCES += AssertBoost.C
libLintel_la_LIBADD += -lboost_signals
endif

if WITH_TCL
BUILT_SOURCES = LintelTclCommands.C
libLintel_la_SOURCES += LintelTclCommands.C Nameable.C \
  Random.C Randommwm.C TclInterface.C 

# Headers for classes which define Tcl-accesible functions:
TCL_HDRS = $(top_srcdir)/include/Lintel/Random.H

# TODO: I think this is a broken way of doing things and that we should
# always rebuild LintelTclCommands.C into the build directory

LintelTclCommands.C: $(top_srcdir)/include/Lintel/TclInterface.H $(TCL_HDRS) $(top_builddir)/src/buildTcl
	@[ ! -f $(top_srcdir)/src/LintelTclCommands.C ] || rm $(top_srcdir)/src/LintelTclCommands.C
	$(top_builddir)/src/buildTcl -- -o $@-new -t $(TCL_HDRS)
	@echo "int UseLintelTclCommands = 1; /* here to force linking */" >>$@-new
	mv $@-new $@
endif

if WITH_GC
libLintelGC_la_LDFLAGS = -version-info @LIBLINTELGC_VERSION@ 
libLintelGC_la_LIBADD = libLintel.la
libLintelGC_la_SOURCES = \
	StringId.C \
	ConstantString.C

libnogc_la_LDFLAGS = -version-info @LIBNOGC_VERSION@
libnogc_la_SOURCES = nogc.c 
endif

if WITH_BOOST
libLintelPThread_la_LDFLAGS = -version-info @LIBLINTELPTHREAD_VERSION@ 
libLintelPThread_la_LIBADD = libLintel.la -lpthread
libLintelPThread_la_SOURCES = \
	PThread.C
endif

if WITH_MAKEBITS
install-exec-local::
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	$(install_sh_DATA) $(srcdir)/Make.common $(DESTDIR)$(libdir)/Make.common
	$(install_sh_DATA) $(srcdir)/Make.world $(DESTDIR)$(libdir)/Make.world
endif

bpmodulesdir = $(datadir)/bp_modules
nobase_bpmodules_DATA = BatchParallel/common.pm BatchParallel/compress.pm BatchParallel/jobsfile.pm

perllibdir = $(datadir)/perl5
nobase_perllib_DATA = Text/Expand.pm Plot/Mercury.pm Plot/Mercury/Tics.pm Lintel/Net/SSH/KnownHostsFile.pm

packagingdir = $(datadir)/packaging
packaging_DATA = redhat-rules

## TODO: ought to move all this regression stuff to a dejagnu style
## regression test, which is hopefully sufficiently flexible to handle
## the occasionally complex regression tests that we perform.

check_PROGRAMS := tests/check-Stats tests/check-StatsQuantile tests/check-Uncompress tests/check-LintelAssert tests/check-Clock tests/check-hashtable tests/check-mersenneTwister

if WITH_TCL
check_PROGRAMS += tests/check-Random
endif

check-local: tests/checked_Stats tests/checked_StatsQuantile tests/checked_Uncompress tests/checked_LintelAssert tests/checked_Clock tests/checked_hashtable tests/checked_mersenneTwister
	@echo "Regression tests successful"

clean-local::
	-rm -f tests/checked_*

if WITH_TCL
tests_check_Random_SOURCES = tests/check-Random.C

tests/checked_Random: tests/check-Random
	tests/check-Random 2>&1 | diff -b $(srcdir)/@RANDOM_COMPARETO@ -
	touch $@
endif

tests_check_Stats_SOURCES = tests/stats.cpp

tests/checked_Stats: tests/check-Stats
	tests/check-Stats
	touch $@

tests_check_StatsQuantile_SOURCES = tests/stats_quantile.C

tests/checked_StatsQuantile: tests/check-StatsQuantile
	tests/check-StatsQuantile | diff -c $(srcdir)/tests/stats_quantile.good -
	touch $@

tests_check_Uncompress_SOURCES = tests/check-Uncompress.C

tests/checked_Uncompress: tests/check-Uncompress
	tests/check-Uncompress $(srcdir)/Uncompress.C | diff -c $(srcdir)/Uncompress.C -
	gzip -1cv <$(srcdir)/Uncompress.C >tests/Uncompress.C.gz
	tests/check-Uncompress tests/Uncompress.C.gz | diff -c $(srcdir)/Uncompress.C -
	rm tests/Uncompress.C.gz
	touch $@

tests_check_LintelAssert_SOURCES = tests/check-LintelAssert.C

tests/checked_LintelAssert: tests/check-LintelAssert $(srcdir)/tests/run-check-LintelAssert.sh
	$(srcdir)/tests/run-check-LintelAssert.sh $(srcdir)
	touch $@

tests_check_Clock_SOURCES = tests/clock.C

tests/checked_Clock: tests/check-Clock
	tests/check-Clock
	touch $@

tests_check_hashtable_SOURCES = tests/hashtable.C

tests/checked_hashtable: tests/check-hashtable
	tests/check-hashtable
	touch $@

tests_check_mersenneTwister_SOURCES = tests/mersenneTwister.C

tests/checked_mersenneTwister: tests/check-mersenneTwister
	tests/check-mersenneTwister
	touch $@
