#
# (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake description for the include directory

SET(INCLUDE_FILES
	AssertBoost.hpp
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
	LintelLog.hpp
	LintelVersion.hpp
	MathSpecialFunctions.hpp
	Matrix.hpp
	MersenneTwisterRandom.hpp
	PointerUtil.hpp
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
	AssertBoost.H
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

IF(ENABLE_STDIO64) 
    LIST(APPEND INCLUDE_FILES stdio_64.hpp)
    LIST(APPEND TRANSITIONAL_INCLUDE_FILES stdio_64.H)
ENDIF(ENABLE_STDIO64)

IF(TCL_ENABLED)
    LIST(APPEND INCLUDE_FILES 
  	Nameable.hpp
  	Random.hpp
  	Randommwm.hpp
  	TclInterface.hpp)
  
    LIST(APPEND TRANSITIONAL_INCLUDE_FILES 
  	Nameable.H
  	Random.H
  	Randommwm.H
  	TclInterface.H)
ENDIF(TCL_ENABLED)

IF(THREADS_ENABLED)
    LIST(APPEND INCLUDE_FILES PThread.hpp)
    LIST(APPEND TRANSITIONAL_INCLUDE_FILES PThread.H)
ENDIF(THREADS_ENABLED)
  
IF(LIBXML2_ENABLED)
    LIST(APPEND INCLUDE_FILES LintelLogXML.hpp XMLUtil.hpp)
ENDIF(LIBXML2_ENABLED)


################################## INSTALL

INSTALL(FILES ${INCLUDE_FILES} ${TRANSITIONAL_INCLUDE_FILES}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Lintel)
