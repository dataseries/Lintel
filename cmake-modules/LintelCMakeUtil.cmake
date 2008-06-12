MACRO(LINTEL_CONFIG_FILE file_name)
#    MESSAGE("${file_name}: substitute @vars")
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${file_name}.in
                   ${CMAKE_CURRENT_BINARY_DIR}/${file_name}
                   @ONLY)
ENDMACRO(LINTEL_CONFIG_FILE)

MACRO(LINTEL_INSTALL_CONFIG_PROGRAM file_name)
    LINTEL_CONFIG_FILE(${file_name})				   
    INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${file_name}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
ENDMACRO(LINTEL_INSTALL_CONFIG_PROGRAM)

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

# This macro searches for the perl module ${module_name}. It sets
# ${variable}_FOUND to whether the module was found.  It creates a
# configuration variable WITH_${variable} so the user can control
# whether this perl module should be used.  Finally in combines these
# into ${variable}_ENABLED for use in the CMakeLists.txt to determine
# if we should build using this module.

# TODO: make this a little more like the LintelFind macros, and move
# it in there.

MACRO(LINTEL_FIND_PERL_MODULE module_name variable)
    IF("${PERL_FOUND}" STREQUAL "")
        INCLUDE(FindPerl)
    ENDIF("${PERL_FOUND}" STREQUAL "")

    IF("${${variable}_FOUND}" STREQUAL "YES")
        # ... nothing to do, already found it
    ELSEIF("${WITH_${variable}}" STREQUAL "OFF")
        # user has explicitly disabled it; don't bother to check
    ELSE("${${variable}_FOUND}" STREQUAL "YES")
         SET(WITH_${variable} "ON" CACHE BOOL "Enable use of the ${module_name} perl module")
         SET(LFPM_found NO)
         IF(PERL_FOUND)
	     # Tried OUTPUT_QUIET and ERROR_QUIET but with cmake 2.4-patch 5 
	     # this didn't seem to make it quiet.
             EXEC_PROGRAM(${PERL_EXECUTABLE}
                          ARGS -e "\"use lib '${CMAKE_INSTALL_PREFIX}/share/perl5'; use ${module_name};\""
                          RETURN_VALUE LFPM_return_value
		          OUTPUT_VARIABLE LFPM_output
			  ERROR_VARIABLE LFPM_error_output)
             IF("${LFPM_return_value}" STREQUAL 0)
                 SET(LFPM_found YES)
             ENDIF("${LFPM_return_value}" STREQUAL 0)
         ENDIF(PERL_FOUND)
         SET(${variable}_FOUND ${LFPM_found} CACHE BOOL "Found ${module_name} perl module" FORCE)
         MARK_AS_ADVANCED(${variable}_FOUND)
    ENDIF("${${variable}_FOUND}" STREQUAL "YES")

    IF("${WITH_${variable}}" STREQUAL ON AND ${${variable}_FOUND} STREQUAL "YES")
        SET(${variable}_ENABLED YES)
    ELSE("${WITH_${variable}}" STREQUAL ON AND ${${variable}_FOUND} STREQUAL "YES")
        SET(${variable}_ENABLED NO)
    ENDIF("${WITH_${variable}}" STREQUAL ON AND ${${variable}_FOUND} STREQUAL "YES")

#    MESSAGE("HIYA ${variable} ${${variable}_ENABLED}")
ENDMACRO(LINTEL_FIND_PERL_MODULE)

# need to do this to pick up user installed modules.
# need unshift so that if we have commonly installed perl modules
# and user installed ones that the user installed ones take precedence.
SET(PERL_MODULES_INC_UNSHIFT "BEGIN { unshift(@INC,'${CMAKE_INSTALL_PREFIX}/share/perl5') if grep(\$_ eq '${CMAKE_INSTALL_PREFIX}/share/perl5', @INC) == 0;};")
# need this so that we can regression test without having to install first.
# and can't do it through the standard PERLLIB env variable as that
# would get overridden by the earlier unshift.
# TODO: check to see if setting PERL5LIB with PERL_SHAREDIR and this one works now.
SET(PERL_MODULES_INC_UNSHIFT "${PERL_MODULES_INC_UNSHIFT} BEGIN { unshift(@INC, \$ENV{LINTEL_REGRESSION_TEST_INC_DIR}) if defined \$ENV{LINTEL_REGRESSION_TEST_INC_DIR}; };")

