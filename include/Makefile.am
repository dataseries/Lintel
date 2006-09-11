#
#  (c) Copyright 2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

nobase_include_HEADERS = \
	Lintel/AssertException.H \
	Lintel/AssertBoost.H \
	Lintel/Clock.H \
	Lintel/CompilerMarkup.H \
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
	Lintel/StatsEMA.H \
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

