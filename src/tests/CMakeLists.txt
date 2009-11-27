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

MACRO(LINTEL_SIMPLE_LONG_TEST testname)
  LINTEL_SIMPLE_PROGRAM(${testname})
  ADD_TEST(${testname}-long ./${testname})
ENDMACRO(LINTEL_SIMPLE_LONG_TEST)

################################## TESTS

LINTEL_SIMPLE_TEST(hashtable)
LINTEL_SIMPLE_TEST(hashmap)
LINTEL_SIMPLE_TEST(least_squares)
LINTEL_SIMPLE_TEST(mersenneTwister)
LINTEL_SIMPLE_TEST(pointerutil)
LINTEL_SIMPLE_TEST(rotating_hashmap)
LINTEL_SIMPLE_TEST(simple_mutex)
LINTEL_SIMPLE_TEST(stringutil)
LINTEL_SIMPLE_TEST(stats)
LINTEL_SIMPLE_TEST(stats_sequence)
LINTEL_SIMPLE_TEST(deque)
LINTEL_SIMPLE_TEST(hashfns)
LINTEL_SIMPLE_TEST(tuples)
LINTEL_SIMPLE_TEST(hashtuplestats)
LINTEL_SIMPLE_TEST(statscube)
LINTEL_SIMPLE_TEST(priority_queue)
LINTEL_SIMPLE_TEST(byte_buffer)
LINTEL_SIMPLE_TEST(boyer_moore_horspool)
LINTEL_SIMPLE_TEST(stlutility)

################################## SPECIAL TEST PROGRAMS

LINTEL_SIMPLE_PROGRAM(check-LintelAssert)
ADD_TEST(lintel-assert ${CMAKE_CURRENT_SOURCE_DIR}/run-check-LintelAssert.sh ${CMAKE_CURRENT_SOURCE_DIR})

LINTEL_SIMPLE_PROGRAM(lintel_log)
ADD_TEST(lintel_log ${CMAKE_CURRENT_SOURCE_DIR}/run-check-lintel-log.sh)

LINTEL_SIMPLE_PROGRAM(testutil)
ADD_TEST(test_util ${CMAKE_CURRENT_SOURCE_DIR}/run-check-test-util.sh)

IF(PERL_ENABLED)
    ADD_TEST(deptool-env env PERL5LIB=${Lintel_SOURCE_DIR}/src:${CMAKE_INSTALL_PREFIX}/share/perl5:$ENV{PERL5LIB} ${CMAKE_CURRENT_BINARY_DIR}/../deptool getenv for-sh)
    ADD_TEST(deptool-help-init env PERL5LIB=${Lintel_SOURCE_DIR}/src:${CMAKE_INSTALL_PREFIX}/share/perl5:$ENV{PERL5LIB} ${CMAKE_CURRENT_BINARY_DIR}/../deptool help init)
ENDIF(PERL_ENABLED)

IF(LINTEL_DBI_ENABLED)
    ADD_TEST(test_lintel_dbi ${PERL_PATH} -I${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../perl/test_lintel_dbi.pl)
ENDIF(LINTEL_DBI_ENABLED)


ADD_TEST(byte_buffer byte_buffer ${CMAKE_CURRENT_SOURCE_DIR}/byte-buffer-test-inp)

################################## CONDITIONAL BITS

IF(LIBXML2_ENABLED)
    INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
    LINTEL_SIMPLE_TEST(xmlutil)
    LINTEL_SIMPLE_TEST(lintellogxml)
ENDIF(LIBXML2_ENABLED)

IF(THREADS_ENABLED)
    LINTEL_SIMPLE_TEST(pthread)
    TARGET_LINK_LIBRARIES(pthread LintelPThread)

    LINTEL_SIMPLE_TEST(atomic_counter)
    TARGET_LINK_LIBRARIES(atomic_counter LintelPThread)
ENDIF(THREADS_ENABLED)

IF(LATEX_ENABLED)
    ADD_TEST(lintel-latex-rebuild ${CMAKE_CURRENT_BINARY_DIR}/../lintel-latex-rebuild --selfcheck)
ENDIF(LATEX_ENABLED)

IF(BOOST_PROGRAM_OPTIONS_ENABLED)
    LINTEL_SIMPLE_PROGRAM(program_options)
    ADD_TEST(program_options ${CMAKE_CURRENT_SOURCE_DIR}/run-check-program-options.sh ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(BOOST_PROGRAM_OPTIONS_ENABLED)

IF(MERCURY_PLOT_ENABLED)
    ADD_TEST(mercury-plot ${CMAKE_CURRENT_SOURCE_DIR}/mercury-plot.sh)
ENDIF(MERCURY_PLOT_ENABLED)

################################### LONGER TESTS

LINTEL_SIMPLE_LONG_TEST(stats_quantile stats_quantile-long)

IF(ENABLE_CLOCK_TEST)
     LINTEL_SIMPLE_TEST(clock)
ENDIF(ENABLE_CLOCK_TEST)

ADD_TEST(flock-long ${CMAKE_CURRENT_SOURCE_DIR}/flock.sh ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_INSTALL_PREFIX})

