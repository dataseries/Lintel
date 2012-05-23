#
# (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake description for the include directory

SET(INCLUDE_FILES
	AssertBoost.hpp
	AssertException.hpp
	Base64.hpp
	BoyerMooreHorspool.hpp
	ByteBuffer.hpp
	Clock.hpp
	CompilerMarkup.hpp
	ConstantString.hpp
	DebugFlag.hpp
	Deque.hpp
	Double.hpp
        FileUtil.hpp
	HashFns.hpp
	HashMap.hpp
	HashTable.hpp
	HashTupleStats.hpp
	HashUnique.hpp
        InterpolationSearch.hpp
	LeastSquares.hpp
	LintelLog.hpp
	LintelVersion.hpp
	LockOrderingGroup.hpp
	MathSpecialFunctions.hpp
	Matrix.hpp
        MarsagliaRandom.hpp
	MersenneTwisterRandom.hpp
	PointerUtil.hpp
	Posix.hpp
	PriorityQueue.hpp
        RandomBase.hpp
	RotatingHashMap.hpp
	SimpleMutex.hpp
	STLUtility.hpp
	Stats.hpp
	StatsCube.hpp
	StatsEMA.hpp
	StatsHistogram.hpp
	StatsMaker.hpp
	StatsQuantile.hpp
	StatsRW.hpp
	StatsSequence.hpp
	StatsSeries.hpp
	StatsSeriesGroup.hpp
	StringUtil.hpp
	TestUtil.hpp
	Tuples.hpp
        TypeCompat.hpp
)

################################## CONDITIONAL BITS

IF(ENABLE_STDIO64) 
    LIST(APPEND INCLUDE_FILES stdio_64.hpp)
ENDIF(ENABLE_STDIO64)

IF(THREADS_ENABLED)
    LIST(APPEND INCLUDE_FILES ${CMAKE_CURRENT_BINARY_DIR}/PThread.hpp AtomicCounter.hpp)
ENDIF(THREADS_ENABLED)
  
IF(LIBXML2_ENABLED)
    LIST(APPEND INCLUDE_FILES LintelLogXML.hpp XMLUtil.hpp)
ENDIF(LIBXML2_ENABLED)

IF(BOOST_PROGRAM_OPTIONS_ENABLED)
    LIST(APPEND INCLUDE_FILES ProgramOptions.hpp)
ENDIF(BOOST_PROGRAM_OPTIONS_ENABLED)

################################## INSTALL

INSTALL(FILES ${INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Lintel)

ADD_SUBDIRECTORY(unstable)
