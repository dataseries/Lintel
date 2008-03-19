#
#  (c) Copyright 2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

nobase_include_HEADERS = \
	Lintel/AssertException.hpp \
	Lintel/Clock.hpp \
	Lintel/CompilerMarkup.hpp \
	Lintel/ConstantString.hpp \
	Lintel/Deque.hpp \
	Lintel/Double.hpp \
	Lintel/HashMap.hpp \
	Lintel/HashTable.hpp \
	Lintel/HashUnique.hpp \
	Lintel/LeastSquares.hpp \
	Lintel/LintelAssert.hpp \
	Lintel/LintelVersion.hpp \
	Lintel/MathSpecialFunctions.hpp \
	Lintel/Matrix.hpp \
	Lintel/MersenneTwisterRandom.hpp \
	Lintel/PThread.hpp \
	Lintel/Posix.hpp \
	Lintel/PriorityQueue.hpp \
	Lintel/RotatingHashMap.hpp \
	Lintel/Stats.hpp \
	Lintel/StatsEMA.hpp \
	Lintel/StatsHistogram.hpp \
	Lintel/StatsMaker.hpp \
	Lintel/StatsQuantile.hpp \
	Lintel/StatsRW.hpp \
	Lintel/StatsSequence.hpp \
	Lintel/StatsSeries.hpp \
	Lintel/StatsSeriesGroup.hpp \
	Lintel/StringUtil.hpp \
	Lintel/Uncompress.hpp \
	\
	Lintel/AssertException.H \
	Lintel/Clock.H \
	Lintel/CompilerMarkup.H \
	Lintel/ConstantString.H \
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
	Lintel/streamcompat.H \
	Lintel/StringUtil.H \
	Lintel/Uncompress.H 

if ENABLE_STDIO64
nobase_include_HEADERS += Lintel/stdio_64.hpp Lintel/stdio_64.H
endif

if WITH_BOOST
nobase_include_HEADERS += Lintel/AssertBoost.hpp Lintel/AssertBoost.H
endif

if WITH_TCL
nobase_include_HEADERS += \
	Lintel/Nameable.hpp \
	Lintel/Random.hpp \
	Lintel/Randommwm.hpp \
	Lintel/TclInterface.hpp \
	\
	Lintel/Nameable.H \
	Lintel/Random.H \
	Lintel/Randommwm.H \
	Lintel/TclInterface.H
endif

if WITH_GC
nobase_include_HEADERS +=
	Lintel/GC_Alloc.hpp Lintel/GC_Alloc.H

Lintel/GC_Alloc.hpp: make_gcalloc_h.pl
	[ -d Lintel ] || mkdir Lintel
	perl $(srcdir)/make_gcalloc_h.pl >$@-new
	mv $@-new $@

endif

if WITH_HPUX_REPLACE_MEMORY
nobase_include_HEADERS += Lintel/hp_replace/memory
endif

