#
# (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake rules for the src directory

INCLUDE_DIRECTORIES(
	${Lintel_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include)

ADD_DEFINITIONS(-DLIBLINTEL_VERSION="\\"${LINTEL_VERSION}\\"")

SET(LIBLINTEL_SOURCES
	AssertException.cpp
	Clock.cpp
	ConstantString.cpp
	Deque.cpp
	Double.cpp
	HashTable.cpp
	LeastSquares.cpp
	LintelAssert.cpp
	LintelVersion.cpp
	MathSpecialFunctions.cpp
	Matrix.cpp
	MersenneTwisterRandom.cpp
	PriorityQueue.cpp
	Stats.cpp
	StatsEMA.cpp
	StatsHistogram.cpp
	StatsMaker.cpp
	StatsRW.cpp
	StatsQuantile.cpp
	StatsSequence.cpp
	StatsSeries.cpp
	StatsSeriesGroup.cpp
	StringUtil.cpp
	Uncompress.cpp
)

SET(LINTEL_FEATURES boost)

################################## SUBDIRS

ADD_SUBDIRECTORY(BatchParallel)

################################## PROGRAMS

MACRO(LINTEL_SIMPLE_PROGRAM program_name)
  ADD_EXECUTABLE(${program_name} ${program_name}.cpp)
  ADD_DEPENDENCIES(${program_name} Lintel)
  TARGET_LINK_LIBRARIES(${program_name} Lintel)
ENDMACRO(LINTEL_SIMPLE_PROGRAM)

LINTEL_SIMPLE_PROGRAM(calcStats)
LINTEL_SIMPLE_PROGRAM(drawRandomLogNormal)

ADD_SUBDIRECTORY(tests)

################################## PERL MODULES

LINTEL_INSTALL_FILE_PATH(${CMAKE_INSTALL_PREFIX}/share/perl5
	Text/Expand.pm
	Text/ExpandInt.pm
	Plot/Mercury.pm
	Plot/Mercury/Tics.pm
	Lintel/Net/SSH/KnownHostsFile.pm	
)

################################## CONDITIONAL BITS

IF(ENABLE_STDIO64)
  LIST(APPEND LINTEL_FEATURES stdio64)
  SET(LIBLINTEL_SOURCES ${LIBLINTEL_SOURCES} stdio_64.cpp)
ENDIF(ENABLE_STDIO64)

SET(LIBLINTEL_SOURCES ${LIBLINTEL_SOURCES} AssertBoost.cpp)

IF(THREADS_ENABLED)
  LIST(APPEND LINTEL_FEATURES threads)
  SET(LIBLINTELPTHREAD_SOURCES PThread.cpp ClockPThread.cpp)
ENDIF(THREADS_ENABLED)

IF(TCL_ENABLED)
  LIST(APPEND LINTEL_FEATURES tcl)
  SET(TCL_HEADERS ${Lintel_SOURCE_DIR}/include/Lintel/Random.hpp)
  INCLUDE_DIRECTORIES(${TCL_INCLUDE_PATH})

  LINTEL_INSTALL_CONFIG_PROGRAM(buildTcl)
  ADD_CUSTOM_COMMAND(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/LintelTclCommands.cpp
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/buildTcl
    ARGS -- -o ${CMAKE_CURRENT_BINARY_DIR}/LintelTclCommands.cpp -t ${TCL_HEADERS}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/buildTcl ${TCL_HEADERS}
  )

  SET(LIBLINTEL_SOURCES ${LIBLINTEL_SOURCES}
	${CMAKE_CURRENT_BINARY_DIR}/LintelTclCommands.cpp
	Nameable.cpp
	Random.cpp
	Randommwm.cpp
	TclInterface.cpp)
ENDIF(TCL_ENABLED)

################################## LIBRARY

ADD_LIBRARY(Lintel ${LIBRARY_TYPE} ${LIBLINTEL_SOURCES})
SET_TARGET_PROPERTIES(Lintel 
  PROPERTIES VERSION ${LINTEL_VERSION} SONAME ${LINTEL_ABI_VERSION})

################################## CONDITIONAL LIBRARY

IF(TCL_ENABLED) 
TARGET_LINK_LIBRARIES(Lintel ${TCL_LIBRARY})
ENDIF(TCL_ENABLED)

IF(THREADS_ENABLED)
  ADD_LIBRARY(LintelPThread ${LIBRARY_TYPE} ${LIBLINTELPTHREAD_SOURCES})
  ADD_DEPENDENCIES(LintelPThread Lintel)
  SET_TARGET_PROPERTIES(LintelPThread
    PROPERTIES VERSION ${LINTEL_VERSION} SONAME ${LINTEL_ABI_VERSION})
  TARGET_LINK_LIBRARIES(LintelPThread Lintel ${CMAKE_THREAD_LIBS_INIT})
ENDIF(THREADS_ENABLED)

################################## INSTALL

INSTALL(TARGETS calcStats drawRandomLogNormal
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
INSTALL(TARGETS Lintel DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
LINTEL_INSTALL_CONFIG_PROGRAM(batch-parallel)
LINTEL_INSTALL_CONFIG_PROGRAM(mercury-plot)
LINTEL_INSTALL_CONFIG_PROGRAM(lintel-config)
LINTEL_INSTALL_CONFIG_PROGRAM(lintel-acinclude)
LINTEL_INSTALL_CONFIG_FILE(redhat-rules 
	${CMAKE_INSTALL_PREFIX}/share/packaging)

IF(THREADS_ENABLED)
  INSTALL(TARGETS LintelPThread DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
ENDIF(THREADS_ENABLED)