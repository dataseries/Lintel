#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake rules for the src/tests directory

################################## MACROS

MACRO(LINTEL_ENVTEST testname)
ENDMACRO(LINTEL_ENVTEST)

MACRO(LINTEL_SIMPLE_TEST testname)
  LINTEL_SIMPLE_PROGRAM(${testname})
  ADD_TEST(${testname} ./${testname})
ENDMACRO(LINTEL_SIMPLE_TEST)

################################## SPECIAL TEST PROGRAMS

LINTEL_SIMPLE_PROGRAM(check-LintelAssert)
LINTEL_SIMPLE_PROGRAM(check-Uncompress)
LINTEL_SIMPLE_PROGRAM(lintel_log)

################################## TESTS

LINTEL_SIMPLE_TEST(stats)
LINTEL_SIMPLE_TEST(stats_quantile)
LINTEL_SIMPLE_TEST(stats_sequence)
LINTEL_SIMPLE_TEST(clock)
LINTEL_SIMPLE_TEST(hashtable)
LINTEL_SIMPLE_TEST(mersenneTwister)
LINTEL_SIMPLE_TEST(least_squares)
LINTEL_SIMPLE_TEST(rotating_hashmap)
LINTEL_SIMPLE_TEST(simple_mutex)
LINTEL_SIMPLE_TEST(pointerutil)

ADD_TEST(lintel-assert ${CMAKE_CURRENT_SOURCE_DIR}/run-check-LintelAssert.sh ${CMAKE_CURRENT_SOURCE_DIR})
ADD_TEST(lintel-uncompress ${CMAKE_CURRENT_SOURCE_DIR}/run-check-Uncompress.sh ${CMAKE_CURRENT_SOURCE_DIR})
ADD_TEST(lintel_log ${CMAKE_CURRENT_SOURCE_DIR}/run-check-lintel-log.sh)

################################## CONDITIONAL BITS

IF(TCL_ENABLED)
    INCLUDE_DIRECTORIES(${TCL_INCLUDE_PATH})
    LINTEL_SIMPLE_PROGRAM(check-Random)
    LINTEL_ENVTEST(lintel-random ${CMAKE_CURRENT_SOURCE_DIR}/run-check-Random.sh ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(TCL_ENABLED)

IF(LIBXML2_ENABLED)
    INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
    LINTEL_SIMPLE_TEST(xmlutil)
    LINTEL_SIMPLE_TEST(lintellogxml)
ENDIF(LIBXML2_ENABLED)

