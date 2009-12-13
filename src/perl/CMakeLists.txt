# If we don't handle all the compilation details in cmake, then we don't get
# the magic re-linking to change the link paths on installation that cmake
# does.

# TODO: figure out how to generalize this so that it can be used in other packages.

IF(NOT PERL_XSUBPPDIR)
    EXECUTE_PROCESS(
        COMMAND perl -e "for (@INC) { next unless -r qq{\$_/ExtUtils/xsubpp}; print; exit(0); }; print q{NOT-FOUND};"
	OUTPUT_VARIABLE PERL_TMP)
    IF(NOT EXISTS "${PERL_TMP}/ExtUtils/xsubpp")
        MESSAGE(FATAL_ERROR "Can not find ExtUtils/xsubpp in perl @INC dirs")
    ENDIF(NOT EXISTS "${PERL_TMP}/ExtUtils/xsubpp")

    SET(PERL_XSUBPPDIR "${PERL_TMP}/ExtUtils" CACHE INTERNAL "directory containing xsubpp")
ENDIF(NOT PERL_XSUBPPDIR)

IF(NOT PERL_COREINCDIR)
    EXECUTE_PROCESS(COMMAND perl -e "use Config; print \$Config{installarchlib};"
                    OUTPUT_VARIABLE PERL_TMP)
    IF(NOT EXISTS "${PERL_TMP}/CORE/perl.h")
        MESSAGE(FATAL_ERROR "Can not find ${PERL_TMP}/perl.h in perl installarchlib dir")
    ENDIF(NOT EXISTS "${PERL_TMP}/CORE/perl.h")

    SET(PERL_COREINCDIR "${PERL_TMP}/CORE" CACHE INTERNAL "directory containing perl.h")
ENDIF(NOT PERL_COREINCDIR)
  
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/LintelPerl.cpp
                   COMMAND perl ${PERL_XSUBPPDIR}/xsubpp -typemap ${PERL_XSUBPPDIR}/typemap 
		           -typemap ${CMAKE_CURRENT_SOURCE_DIR}/typemap
			   ${CMAKE_CURRENT_SOURCE_DIR}/LintelPerl.xs >LintelPerl-1
	           COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/xsubpp-cleanup < LintelPerl-1 >LintelPerl-2
		   COMMAND rm LintelPerl-1
	           COMMAND mv LintelPerl-2 LintelPerl.cpp
		   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/xsubpp-cleanup ${CMAKE_CURRENT_SOURCE_DIR}/LintelPerl.xs)

SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/LintelPerl.cpp
                            PROPERTIES GENERATED true
			    COMPILE_FLAGS "-I${PERL_COREINCDIR}")

ADD_LIBRARY(LintelPerl SHARED ${CMAKE_CURRENT_BINARY_DIR}/LintelPerl.cpp)
TARGET_LINK_LIBRARIES(LintelPerl Lintel)

INSTALL(TARGETS LintelPerl DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/perl5)
INSTALL(FILES Lintel.pm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/perl5)

ADD_TEST(lintel-perl env PERL5LIB=${CMAKE_CURRENT_BINARY_DIR}:${CMAKE_CURRENT_SOURCE_DIR}:$ENV{PERL5LIB} perl ${CMAKE_CURRENT_SOURCE_DIR}/lintel.pl)
