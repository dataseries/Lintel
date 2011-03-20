MACRO(LINTEL_CONFIG_FILE file_name)
#    MESSAGE("${file_name}: substitute @vars")
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${file_name}.in
                   ${CMAKE_CURRENT_BINARY_DIR}/${file_name}
                   @ONLY)
ENDMACRO(LINTEL_CONFIG_FILE)

MACRO(LINTEL_INSTALL_CONFIG_PROGRAM file_name)
    LINTEL_CONFIG_FILE(${file_name})
    EXECUTE_PROCESS(COMMAND chmod +x ${CMAKE_CURRENT_BINARY_DIR}/${file_name})
    INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${file_name}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
ENDMACRO(LINTEL_INSTALL_CONFIG_PROGRAM)

MACRO(LINTEL_INSTALL_CONFIG_POD2MAN_PROGRAM file_name)
    LINTEL_INSTALL_CONFIG_PROGRAM(${file_name})
    LINTEL_POD2MAN(${file_name} ${ARGN})
ENDMACRO(LINTEL_INSTALL_CONFIG_POD2MAN_PROGRAM)

MACRO(LINTEL_INSTALL_CONFIG_FILE file_name dest_prefix)
    LINTEL_CONFIG_FILE(${file_name})
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${file_name}
            DESTINATION ${dest_prefix})
ENDMACRO(LINTEL_INSTALL_CONFIG_FILE)

MACRO(LINTEL_INSTALL_ONE_FILE_PATH path_name dest_prefix)
    GET_FILENAME_COMPONENT(tmp ${path_name} PATH)
    INSTALL(FILES ${path_name}
  	  DESTINATION ${dest_prefix}/${tmp})
ENDMACRO(LINTEL_INSTALL_ONE_FILE_PATH)

# prefix file ...
MACRO(LINTEL_INSTALL_FILE_PATH dest_prefix)
    FOREACH(file ${ARGN})
        LINTEL_INSTALL_ONE_FILE_PATH(${file} ${dest_prefix})
    ENDFOREACH(file)
ENDMACRO(LINTEL_INSTALL_FILE_PATH)

# need to do this to pick up user installed modules.
# need unshift so that if we have commonly installed perl modules
# and user installed ones that the user installed ones take precedence.
#
# to write a test that will work before used modules are installed,
# you want something like:
# ADD_TEST(test-name env PERL5LIB=${Lintel_SOURCE_DIR}/src/perl-modules:${CMAKE_INSTALL_PREFIX}/share/perl5:$ENV{PERL5LIB} program --args)

# Intentionally crammed onto a single line so that line number doesn't change
# when you put this bit into the .in files.  Use prefix/lib/perl5 since that
# directory shouldn't be shared between architectures, also consistent with
# location used by debian lenny, which also uses includes /version for both the
# share and lib dirs.
SET(PERL5_MODULES_INC_UNSHIFT "BEGIN { foreach my \$dir (qw'${CMAKE_INSTALL_PREFIX}/share/perl5 ${CMAKE_INSTALL_PREFIX}/lib/perl5') { unshift(@INC, \$dir) if grep(\$_ eq \$dir, @INC) == 0;}; }")

### Try to compile and run some command; set var to TRUE if
### successfully run, and FALSE otherwise.
### Usage: LINTEL_TRY_RUN(variable source_file <TRY_RUN extra arguments>)
### e.g. LINTEL_TRY_RUN(LINTEL_ISLOCKED linux-islocked.cpp 
###                     CMAKE_FLAGS -DLINK_LIBRARIES=-lpthread)

MACRO(LINTEL_TRY_RUN variable source_file)
    TRY_RUN(${variable}_RUN ${variable}_COMPILE
	    ${CMAKE_CURRENT_BINARY_DIR}
	    ${CMAKE_CURRENT_SOURCE_DIR}/${source_file}
	    ${ARGN})
    IF("${variable}_RUN" STREQUAL "0" 
       AND "${variable}_COMPILE" STREQUAL "TRUE")
       SET(${variable} TRUE)
    ELSE("${variable}_RUN" STREQUAL "0" 
         AND "${variable}_COMPILE" STREQUAL "TRUE")
       SET(${variable} FALSE)
    ENDIF("${variable}_RUN" STREQUAL "0" 
          AND "${variable}_COMPILE" STREQUAL "TRUE")
ENDMACRO(LINTEL_TRY_RUN)

### Set up the rpath stuff the way we seem to always do it.

MACRO(LINTEL_RPATH_CONFIG)
    OPTION(WITH_INSTALLED_RPATH "Install with the rpath set so you will not need to set \$LD_LIBRARY_PATH" ON)
    
    IF(WITH_INSTALLED_RPATH)
        # use, i.e. don't skip the full RPATH for the build tree
        SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
      
        # when building, don't use the install RPATH already
        # (but later on when installing)
        SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
      
        # the RPATH to be used when installing
        SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
      
        # add the automatically determined parts of the RPATH
        # which point to directories outside the build tree to the install RPATH
        SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    ENDIF(WITH_INSTALLED_RPATH)
ENDMACRO(LINTEL_RPATH_CONFIG)

