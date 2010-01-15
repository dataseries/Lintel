# If we don't handle all the compilation details in cmake, then we don't get
# the magic re-linking to change the link paths on installation that cmake
# does.

# TODO: figure out how to generalize this so that it can be used in other packages.

IF(NOT PERL_XSUBPPDIR)
    EXECUTE_PROCESS(
        COMMAND perl -e "for (@INC) { next unless -r qq{\$_/ExtUtils/xsubpp}; print; exit(0); }; print q{NOT-FOUND};"
	OUTPUT_VARIABLE PERL_TMP)
    IF(EXISTS "${PERL_TMP}/ExtUtils/xsubpp")
        SET(PERL_XSUBPPDIR "${PERL_TMP}/ExtUtils" CACHE INTERNAL "directory containing xsubpp")
    ELSE(EXISTS "${PERL_TMP}/ExtUtils/xsubpp")
        MESSAGE("Can NOT build binary Perl modules: ExtUtils/xsubpp not found in perl @INC dirs")
    ENDIF(EXISTS "${PERL_TMP}/ExtUtils/xsubpp")
ENDIF(NOT PERL_XSUBPPDIR)

IF(NOT PERL_COREINCDIR)
    EXECUTE_PROCESS(COMMAND perl -e "use Config; print \$Config{installarchlib};"
                    OUTPUT_VARIABLE PERL_TMP)
    IF(EXISTS "${PERL_TMP}/CORE/perl.h")
        SET(PERL_COREINCDIR "${PERL_TMP}/CORE" CACHE INTERNAL "directory containing perl.h")
    ELSE(EXISTS "${PERL_TMP}/CORE/perl.h")
        MESSAGE("Can NOT build binary Perl modules: perl.h not found as")
	MESSAGE("  ${PERL_TMP}/CORE/perl.h")
    ENDIF(EXISTS "${PERL_TMP}/CORE/perl.h")
ENDIF(NOT PERL_COREINCDIR)

IF(PERL_XSUBPPDIR AND PERL_COREINCDIR)
    SET(PERL_BINARY_MODULES_ENABLED YES)
ELSE(PERL_XSUBPPDIR AND PERL_COREINCDIR)
    SET(PERL_BINARY_MODULES_ENABLED NO)
    LIST(APPEND LINTEL_FIND_ALL_NOTFOUND perl-binary-modules)
ENDIF(PERL_XSUBPPDIR AND PERL_COREINCDIR)
  
IF(PERL_BINARY_MODULES_ENABLED)
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
ENDIF(PERL_BINARY_MODULES_ENABLED)
