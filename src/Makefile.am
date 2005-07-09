## Process this file with automake to produce Makefile.in
###############################################################################
#
# File:         Makefile.am
# RCS:          $Header: /mount/cello/cvs/Lintel/src/Makefile.am,v 1.1 2005/02/14 04:36:52 anderse Exp $
# Description:  Automake input file to generate Makefile.in for configure
# Author:       Eric Anderson
# Created:      Fri Dec 24 14:09:20 2004
# Modified:     Wed Feb  2 12:05:19 2005 (Eric Anderson) anderse@hpl.hp.com
# Language:     Makefile
# Package:      N/A
# Status:       Experimental (Do Not Distribute)
#
# (C) Copyright 2004, Hewlett-Packard Laboratories, all rights reserved.
#
###############################################################################

# avoid requiring all GNUisms
AUTOMAKE_OPTIONS = 1.7 foreign

bin_PROGRAMS = calcStats drawRandomLogNormal 
bin_SCRIPTS = buildTcl batch-parallel lintel-config

calcStats_SOURCES = calcStats.C
drawRandomLogNormal_SOURCES = drawRandomLogNormal.C

LDADD = $(top_builddir)/src/libLintel.la @TCL_LIBS@ libnogc.la -lpthread
AM_CPPFLAGS = -I$(top_srcdir)/include/Lintel -I$(top_builddir)/include/Lintel @TCL_CFLAGS@ @GC_CFLAGS@ -DLIBLINTEL_VERSION="\"@LIBLINTEL_VERSION@\""
AM_LDFLAGS = @OPTMODE_LDFLAGS@

lib_LTLIBRARIES = libLintel.la libnogc.la
libLintel_la_LDFLAGS = -version-info @LIBLINTEL_VERSION@
libLintel_la_LIBADD = @TCL_LIBS@
 
libLintel_la_SOURCES = \
	Clock.C \
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
	StatsHistogram.C \
	StatsMaker.C \
	StatsRW.C \
	StatsQuantile.C \
	StatsSequence.C \
	StatsSeries.C \
	StatsSeriesGroup.C \
	StringUtil.C \
	stdio_64.C \
	Uncompress.C

if WITH_TCL
BUILT_SOURCES = LintelTclCommands.C
libLintel_la_SOURCES += LintelTclCommands.C Nameable.C \
  Random.C Randommwm.C TclInterface.C 

# Headers for classes which define Tcl-accesible functions:
TCL_HDRS = $(top_srcdir)/include/Lintel/Random.H

# TODO: I think this is a broken way of doing things and that we should
# always rebuild LintelTclCommands.C into the build directory

LintelTclCommands.C: $(top_srcdir)/include/Lintel/TclInterface.H $(TCL_HDRS) $(top_srcdir)/src/buildTcl
	@[ ! -f $(top_srcdir)/src/LintelTclCommands.C ] || rm $(top_srcdir)/src/LintelTclCommands.C
	$(top_srcdir)/src/buildTcl -- -o $@-new -t $(TCL_HDRS)
	@echo "int UseLintelTclCommands = 1; /* here to force linking */" >>$@-new
	mv $@-new $@
endif

if WITH_GC
libLintel_la_SOURCES += ConstantString.C
endif

libnogc_la_LDFLAGS = -version-info @LIBNOGC_VERSION@
libnogc_la_SOURCES = nogc.c 

if WITH_MAKEBITS
install-exec-local::
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	$(install_sh_DATA) $(srcdir)/Make.common $(DESTDIR)$(libdir)/Make.common
	$(install_sh_DATA) $(srcdir)/Make.world $(DESTDIR)$(libdir)/Make.world
	$(install_sh_DATA) $(srcdir)/Make.cvs $(DESTDIR)$(libdir)/Make.cvs
endif

bpmodulesdir = $(datadir)/bp_modules
nobase_bpmodules_DATA = BatchParallel/common.pm BatchParallel/compress.pm BatchParallel/jobsfile.pm

# batch-parallel: batch-parallel.in
# 	  cd $(top_builddir) && $(SHELL) ./config.status src/$@

## TODO: ought to move all this regression stuff to a dejagnu style
## regression test, which is hopefully sufficiently flexible to handle
## the occasionally complex regression tests that we perform.

check-local: tests/check_Random tests/check_StatsQuantile tests/check_Uncompress tests/check_LintelAssert
	@echo "Regression tests successful"

clean-local::
	-rm -f tests/check_Random tests/check_StatsQuantile tests/check_Uncompress tests/check_LintelAssert

check_PROGRAMS = tests/check-Random tests/check-StatsQuantile \
	tests/check-Uncompress tests/check-LintelAssert

tests_check_Random_SOURCES = tests/check-Random.C

tests/check_Random: tests/check-Random
	tests/check-Random 2>&1 | diff -b $(srcdir)/@RANDOM_COMPARETO@ -
	touch $@

tests_check_StatsQuantile_SOURCES = tests/stats_quantile.C

tests/check_StatsQuantile: tests/check-StatsQuantile
	tests/check-StatsQuantile | diff -c $(srcdir)/tests/stats_quantile.good -
	touch $@

tests_check_Uncompress_SOURCES = tests/check-Uncompress.C

tests/check_Uncompress: tests/check-Uncompress
	tests/check-Uncompress $(srcdir)/Uncompress.C | diff -c $(srcdir)/Uncompress.C -
	gzip -1cv <$(srcdir)/Uncompress.C >tests/Uncompress.C.gz
	tests/check-Uncompress tests/Uncompress.C.gz | diff -c $(srcdir)/Uncompress.C -
	rm tests/Uncompress.C.gz
	touch $@

tests_check_LintelAssert_SOURCES = tests/check-LintelAssert.C

tests/check_LintelAssert: tests/check-LintelAssert $(srcdir)/tests/run-check-LintelAssert.sh
	$(srcdir)/tests/run-check-LintelAssert.sh $(srcdir)
	touch $@
