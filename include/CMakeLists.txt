#
# (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake description for the include directory

# TODO: move this into Lintel/; remove all the unneeded paths

SET(INCLUDE_FILES
	Lintel/AssertException.hpp
	Lintel/Clock.hpp
	Lintel/CompilerMarkup.hpp
	Lintel/ConstantString.hpp
	Lintel/Deque.hpp
	Lintel/Double.hpp
	Lintel/HashMap.hpp
	Lintel/HashTable.hpp
	Lintel/HashUnique.hpp
	Lintel/LeastSquares.hpp
	Lintel/LintelAssert.hpp
	Lintel/LintelVersion.hpp
	Lintel/MathSpecialFunctions.hpp
	Lintel/Matrix.hpp
	Lintel/MersenneTwisterRandom.hpp
	Lintel/Posix.hpp
	Lintel/PriorityQueue.hpp
	Lintel/RotatingHashMap.hpp
	Lintel/Stats.hpp
	Lintel/StatsEMA.hpp
	Lintel/StatsHistogram.hpp
	Lintel/StatsMaker.hpp
	Lintel/StatsQuantile.hpp
	Lintel/StatsRW.hpp
	Lintel/StatsSequence.hpp
	Lintel/StatsSeries.hpp
	Lintel/StatsSeriesGroup.hpp
	Lintel/StringUtil.hpp
	Lintel/Uncompress.hpp
)

SET(TRANSITIONAL_INCLUDE_FILES
	Lintel/AssertException.H
	Lintel/Clock.H
	Lintel/CompilerMarkup.H
	Lintel/ConstantString.H
	Lintel/Deque.H
	Lintel/Double.H
	Lintel/HashMap.H
	Lintel/HashTable.H
	Lintel/HashUnique.H
	Lintel/LintelAssert.H
	Lintel/LintelVersion.H
	Lintel/MathSpecialFunctions.H
	Lintel/Matrix.H
	Lintel/MersenneTwisterRandom.H
	Lintel/Posix.H
	Lintel/PriorityQueue.H
	Lintel/Stats.H
	Lintel/StatsEMA.H
	Lintel/StatsHistogram.H
	Lintel/StatsMaker.H
	Lintel/StatsQuantile.H
	Lintel/StatsRW.H
	Lintel/StatsSequence.H
	Lintel/StatsSeries.H
	Lintel/StatsSeriesGroup.H
	Lintel/streamcompat.H
	Lintel/StringUtil.H
	Lintel/Uncompress.H 
)

################################## CONDITIONAL BITS

SET(INCLUDE_FILES ${INCLUDE_FILES} Lintel/AssertBoost.hpp)
SET(TRANSITIONAL_INCLUDE_FILES ${TRANSITIONAL_INCLUDE_FILES} Lintel/AssertBoost.H)

IF(ENABLE_STDIO64) 
  SET(INCLUDE_FILES ${INCLUDE_FILES} Lintel/stdio_64.hpp)
  SET(TRANSITIONAL_INCLUDE_FILES ${TRANSITIONAL_INCLUDE_FILES} Lintel/stdio_64.H)
ENDIF(ENABLE_STDIO64)

IF(TCL_ENABLED)
  SET(INCLUDE_FILES ${INCLUDE_FILES} 
	Lintel/Nameable.hpp
	Lintel/Random.hpp
	Lintel/Randommwm.hpp
	Lintel/TclInterface.hpp)

  SET(TRANSITIONAL_INCLUDE_FILES ${TRANSITIONAL_INCLUDE_FILES} 
	Lintel/Nameable.H
	Lintel/Random.H
	Lintel/Randommwm.H
	Lintel/TclInterface.H)
ENDIF(TCL_ENABLED)

IF(THREADS_ENABLED)
  SET(INCLUDE_FILES ${INCLUDE_FILES} Lintel/PThread.hpp)
  SET(TRANSITIONAL_INCLUDE_FILES ${TRANSITIONAL_INCLUDE_FILES} Lintel/PThread.H)
ENDIF(THREADS_ENABLED)
  
################################## INSTALL

INSTALL(FILES ${INCLUDE_FILES} ${TRANSITIONAL_INCLUDE_FILES}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Lintel)
