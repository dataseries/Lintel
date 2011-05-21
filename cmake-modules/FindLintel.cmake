#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# All of the options and dependencies for the various cmake sub-bits
#
# Find the Lintel, LintelPThread includes and library
#
#  set LINTEL_FIND_REQUIRED to require the LINTEL library
#  otherwise, the user will be given a WITH_LINTEL option.
#
#  set LINTELPTHREAD_FIND_REQUIRED to require the LINTELPTHREAD library
#  otherwise, the user will be given a WITH_LINTELPTHREAD option.
#
#  LINTEL_INCLUDE_DIR - where to find Lintel/AssertBoost.hpp
#  LINTEL_LIBRARIES   - List of libraries when using LINTEL
#  LINTEL_ENABLED     - True if LINTEL is enabled
# 
#  LINTELPTHREAD_INCLUDE_DIR - where to find Lintel/PThread.hpp
#  LINTELPTHREAD_LIBRARIES   - List of libraries when using LINTELPTHREAD
#  LINTELPTHREAD_ENABLED     - True if LINTELPTHREAD is enabled

INCLUDE(LintelFind)

LINTEL_BOOST_EXTRA(BOOST_PROGRAM_OPTIONS boost/program_options.hpp 
                   "boost_program_options boost_program_options-mt")
SET(LINTEL_EXTRA_LIBRARIES ${BOOST_PROGRAM_OPTIONS_LIBRARIES})
LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE(LINTEL Lintel/AssertBoost.hpp Lintel)

SET(LINTELPTHREAD_EXTRA_LIBRARIES ${LINTEL_LIBRARIES})
LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE(LINTELPTHREAD Lintel/PThread.hpp LintelPThread)

MACRO(LINTEL_HAS_FEATURE feature)
    IF(NOT LINTEL_${feature}_ENABLED)
        SET(LINTEL_CONFIG_FIND_REQUIRED TRUE)
	LINTEL_FIND_PROGRAM(LINTEL_CONFIG lintel-config)
        EXECUTE_PROCESS(COMMAND ${LINTEL_CONFIG_PATH} --has-feature ${feature}
	                RESULT_VARIABLE lintel_has_feature)
	IF("${lintel_has_feature}" STREQUAL "0") 
	    SET(LINTEL_${feature}_ENABLED TRUE CACHE BOOL 
                "Did lintel support feature ${feature}" FORCE)
	    MARK_AS_ADVANCED(LINTEL_${feature}_ENABLED)
	ELSE("${lintel_has_feature}" STREQUAL "0") 
	    SET(LINTEL_${feature}_ENABLED FALSE)
	ENDIF("${lintel_has_feature}" STREQUAL "0") 
    ENDIF(NOT LINTEL_${feature}_ENABLED)
ENDMACRO(LINTEL_HAS_FEATURE)

