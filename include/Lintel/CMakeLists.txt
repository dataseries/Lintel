#
# (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake description for the include directory

SET(INCLUDE_FILES
	AssertException.hpp
	Clock.hpp
	CompilerMarkup.hpp
	ConstantString.hpp
	Deque.hpp
	Double.hpp
	HashMap.hpp
	HashTable.hpp
	HashUnique.hpp
	LeastSquares.hpp
	LintelAssert.hpp
	LintelVersion.hpp
	MathSpecialFunctions.hpp
	Matrix.hpp
	MersenneTwisterRandom.hpp
	Posix.hpp
	PriorityQueue.hpp
	RotatingHashMap.hpp
	SimpleMutex.hpp
	Stats.hpp
	StatsEMA.hpp
	StatsHistogram.hpp
	StatsMaker.hpp
	StatsQuantile.hpp
	StatsRW.hpp
	StatsSequence.hpp
	StatsSeries.hpp
	StatsSeriesGroup.hpp
	StringUtil.hpp
	Uncompress.hpp
)

SET(TRANSITIONAL_INCLUDE_FILES
	AssertException.H
	Clock.H
	CompilerMarkup.H
	ConstantString.H
	Deque.H
	Double.H
	HashMap.H
	HashTable.H
	HashUnique.H
	LintelAssert.H
	LintelVersion.H
	MathSpecialFunctions.H
	Matrix.H
	MersenneTwisterRandom.H
	Posix.H
	PriorityQueue.H
	Stats.H
	StatsEMA.H
	StatsHistogram.H
	StatsMaker.H
	StatsQuantile.H
	StatsRW.H
	StatsSequence.H
	StatsSeries.H
	StatsSeriesGroup.H
	streamcompat.H
	StringUtil.H
	Uncompress.H 
)

################################## CONDITIONAL BITS

SET(INCLUDE_FILES ${INCLUDE_FILES} AssertBoost.hpp)
SET(TRANSITIONAL_INCLUDE_FILES ${TRANSITIONAL_INCLUDE_FILES} AssertBoost.H)

IF(ENABLE_STDIO64) 
  SET(INCLUDE_FILES ${INCLUDE_FILES} stdio_64.hpp)
  SET(TRANSITIONAL_INCLUDE_FILES ${TRANSITIONAL_INCLUDE_FILES} stdio_64.H)
ENDIF(ENABLE_STDIO64)

IF(TCL_ENABLED)
  SET(INCLUDE_FILES ${INCLUDE_FILES} 
	Nameable.hpp
	Random.hpp
	Randommwm.hpp
	TclInterface.hpp)

  SET(TRANSITIONAL_INCLUDE_FILES ${TRANSITIONAL_INCLUDE_FILES} 
	Nameable.H
	Random.H
	Randommwm.H
	TclInterface.H)
ENDIF(TCL_ENABLED)

IF(THREADS_ENABLED)
  SET(INCLUDE_FILES ${INCLUDE_FILES} PThread.hpp)
  SET(TRANSITIONAL_INCLUDE_FILES ${TRANSITIONAL_INCLUDE_FILES} PThread.H)
ENDIF(THREADS_ENABLED)
  
################################## INSTALL

INSTALL(FILES ${INCLUDE_FILES} ${TRANSITIONAL_INCLUDE_FILES}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Lintel)
