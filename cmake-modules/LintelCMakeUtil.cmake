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

SET(PERL5_MODULES_INC_UNSHIFT "BEGIN { unshift(@INC,'${CMAKE_INSTALL_PREFIX}/share/perl5') if grep(\$_ eq '${CMAKE_INSTALL_PREFIX}/share/perl5', @INC) == 0;};")

SET(PERL_MODULES_INC_UNSHIFT "die 'PERL_MODULES_INC_UNSHIFT is obsolete, use PERL5_MODULES_INC_UNSHIFT';")

# TODO: move this into LintelDocs.cmake

# Set LINTEL_LATEX_REBUILD_REQUIRED to force this to be found
MACRO(LINTEL_LATEX_CONFIG)
    INCLUDE(LintelFind)
    LINTEL_WITH_PROGRAM(LINTEL_LATEX_REBUILD lintel-latex-rebuild)
    IF(LINTEL_LATEX_REBUILD_PATH) 
        EXECUTE_PROCESS(COMMAND ${LINTEL_LATEX_REBUILD_PATH} --selfcheck
	                RESULT_VARIABLE LINTEL_LATEX_REBUILD_SELFCHECK)
	IF(NOT "${LINTEL_LATEX_REBUILD_SELFCHECK}" STREQUAL "0") 
	    IF(LINTEL_LATEX_REBUILD_REQUIRED)
	        MESSAGE(FATAL_ERROR "${LINTEL_LATEX_REBUILD_PATH} --selfcheck failed")
	    ELSE(LINTEL_LATEX_REBUILD_REQUIRED)
	        MESSAGE("${LINTEL_LATEX_REBUILD_PATH} --selfcheck failed (${LINTEL_LATEX_REBUILD_SELFCHECK}); disabling")
	        SET(LINTEL_LATEX_REBUILD_ENABLED)
	    ENDIF(LINTEL_LATEX_REBUILD_REQUIRED)
	ENDIF(NOT "${LINTEL_LATEX_REBUILD_SELFCHECK}" STREQUAL "0") 
    ENDIF(LINTEL_LATEX_REBUILD_PATH)     
ENDMACRO(LINTEL_LATEX_CONFIG)

# Automatically picks up the *.tex, *.eps dependencies; others can be
# specified in ${basename}_EXTRA_DEPENDS; set ${basename}_LINTEL_LATEX_ARGS
# to [--tex <path>] [--bib <path>] to specify extra paths for latex to use.

MACRO(LINTEL_LATEX basename)
    IF(LINTEL_LATEX_REBUILD_ENABLED)
	FILE(GLOB_RECURSE ${basename}_TEX_DEPENDS
	                  ${CMAKE_CURRENT_SOURCE_DIR}/*.tex)
	FILE(GLOB_RECURSE ${basename}_BIB_DEPENDS
	                  ${CMAKE_CURRENT_SOURCE_DIR}/*.bib)
	FILE(GLOB_RECURSE ${basename}_EPS_DEPENDS
	                  ${CMAKE_CURRENT_SOURCE_DIR}/*.eps)
	SET(${basename}_REBUILD_OUTPUTS
	    ${CMAKE_CURRENT_BINARY_DIR}/${basename}.dvi
	    ${CMAKE_CURRENT_BINARY_DIR}/${basename}.ps
            ${CMAKE_CURRENT_BINARY_DIR}/${basename}.pdf)

	ADD_CUSTOM_COMMAND(
	    OUTPUT ${${basename}_REBUILD_OUTPUTS}
            COMMAND ${LINTEL_LATEX_REBUILD_PATH}
	    ARGS
	        ${${basename}_LINTEL_LATEX_ARGS}
		${XYZZY}
                ${CMAKE_CURRENT_SOURCE_DIR} ${basename}
            DEPENDS
		${${basename}_TEX_DEPENDS}
		${${basename}_BIB_DEPENDS}
		${${basename}_EPS_DEPENDS}
	        ${${basename}_EXTRA_DEPENDS}
        )
	ADD_CUSTOM_TARGET(latex_${basename} ALL
	                  DEPENDS ${${basename}_REBUILD_OUTPUTS})
    ENDIF(LINTEL_LATEX_REBUILD_ENABLED)
ENDMACRO(LINTEL_LATEX)

MACRO(LINTEL_LATEX_REQUIRES variable)
    SET(${variable} ${LINTEL_LATEX_REBUILD_ENABLED})
    FOREACH(llr_filename ${ARGN})
	EXECUTE_PROCESS(COMMAND kpsewhich ARGS ${llr_filename}
                        RESULT_VARIABLE llr_result
			OUTPUT_VARIABLE llr_output
			OUTPUT_STRIP_TRAILING_WHITESPACE)
        IF("${llr_output}" STREQUAL "")
	    MESSAGE("Unable to find latex file ${llr_filename} for ${variable}")
	    SET(${variable} FALSE)
        ELSEIF(NOT EXISTS "${llr_output}")
	    MESSAGE("Found ${llr_output} for ${variable} but it does not exist?")
	    SET(${variable} FALSE)
        ENDIF("${llr_output}" STREQUAL "")
    ENDFOREACH(llr_filename)
ENDMACRO(LINTEL_LATEX_REQUIRES)	            

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

