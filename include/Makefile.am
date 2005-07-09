###############################################################################
#
# File:         Makefile.am
# RCS:          $Header: /mount/cello/cvs/Lintel/include/Makefile.am,v 1.1 2005/02/14 04:39:58 anderse Exp $
# Description:  input file for automake
# Author:       Eric Anderson
# Created:      Sat Jan  8 00:16:15 2005
# Modified:     Sun Jan 30 13:15:42 2005 (Eric Anderson) anderse@hpl.hp.com
# Language:     Makefile
# Package:      N/A
# Status:       Experimental (Do Not Distribute)
#
# (C) Copyright 2005, Hewlett-Packard Laboratories, all rights reserved.
#
###############################################################################

nobase_include_HEADERS = \
	Lintel/Clock.H \
	Lintel/Deque.H \
	Lintel/Double.H \
	Lintel/HashMap.H \
	Lintel/HashTable.H \
	Lintel/HashUnique.H \
	Lintel/LintelAssert.H \
	Lintel/LintelVersion.H \
	Lintel/MathSpecialFunctions.H \
	Lintel/Matrix.H \
	Lintel/MersenneTwisterRandom.H \
	Lintel/PThread.H \
	Lintel/Posix.H \
	Lintel/PriorityQueue.H \
	Lintel/Stats.H \
	Lintel/StatsHistogram.H \
	Lintel/StatsMaker.H \
	Lintel/StatsQuantile.H \
	Lintel/StatsRW.H \
	Lintel/StatsSequence.H \
	Lintel/StatsSeries.H \
	Lintel/StatsSeriesGroup.H \
	Lintel/StringUtil.H \
	Lintel/stdio_64.H \
	Lintel/streamcompat.H \
	Lintel/Uncompress.H 

if WITH_TCL
nobase_include_HEADERS += \
	Lintel/Nameable.H \
	Lintel/Random.H \
	Lintel/Randommwm.H \
	Lintel/TclInterface.H
endif

if WITH_GC

nobase_include_HEADERS += \
	Lintel/ConstantString.H \
	Lintel/GC_Alloc.H

Lintel/GC_Alloc.H: make_gcalloc_h.pl
	[ -d Lintel ] || mkdir Lintel
	perl $(srcdir)/make_gcalloc_h.pl >$@-new
	mv $@-new $@
endif

if WITH_HPUX_REPLACE_MEMORY
nobase_include_HEADERS += Lintel/hp_replace/memory
endif

