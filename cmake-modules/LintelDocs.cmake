#
# (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Macros for dealing with documentation, both doxygen and latex

## Doxygen

# Put INCLUDE(LintelDocs), LINTEL_DOCS_CONFIG("your-package-name") in
# your CMakeConfig.txt or wherever you have all the optional bits of
# compilation.

# Put LINTEL_DOCS_BUILD() in your doc/CMakeLists.txt file.  
# Configuration variables: 
#   - DOXYGEN_DEP_EXTRAS: list extra dependencies for the doxygen
#     run.  By default it will list files in project-root/include and
#     build-root/include, although the latter is likely to be flakey.
#   - DOXYGEN_EXAMPLE_PATH: where should examples be found?  Defaults
#     to project-root/src/example
#   - DOXYGEN_EXTRA_INPUTS: where should additional inputs be found?
#     Default list is project-root/include and build-root/include
#   - DOXYGEN_DOTFILE_DIRS: .dot file directories
#     Default is project-root/doc/doxygen-figures

# If you need to customize the doxygen config, copy
# .../share/Lintel/doxygen.config.in to docs/doxygen.config.in, and
# edit as appropriate.  Otherwise the standard input config will be
# used from the Lintel project.

## Latex

# In your main CMakeLists.txt (or CMakeConfig.txt) file, call the macro
# LINTEL_LATEX_CONFIG.  You can set the option LINTEL_LATEX_REBUILD_REQUIRED in
# order to require that the lintel-latex-rebuild script is found.

# For each document, call LINTEL_LATEX(basename).  This will automatically set
# up to build basename.{dvi,ps,pdf} from basename.tex using the complicated
# pattern that has been found to make latex work.  It will also set up the
# environment variables to that the common bibliography (lintel-latex-all.bib,
# and others in doc/references) can be found.  You can set the variable
# basename_EXTRA_DEPENDS to identify extra dependencies, *.tex, *.bib and *.eps
# will be automatically treated as dependencies.  You can set the variable
# basename_LINTEL_LATEX_ARGS to specify extra arguments to
# lintel-latex-rebuild, such as [--tex <path>] [--bib <path>] to specify extra
# paths for latex to use, or include [--paper <paper-size>] to specify a
# non-letter paper size.

# If you require non-standard files, you can call
# LINTEL_LATEX_REQUIRES(variable document_name file...)  to use kpsewhich in
# order to find all of the required files.  If they are all found, and
# lintel-latex-rebuild is present, the variable will be set to TRUE, otherwise
# it will be set to FALSE.  If lintel-latex-rebuild is present and one of the
# files is not, it will print out a message explaining that document_name
# will not be built.

############################################
############### doxygen
############################################

# TODO: rename this to LINTEL_DOXYGEN_CONFIG and deprecate the old one.

MACRO(LINTEL_DOCS_CONFIG in_package_name)
    SET(PACKAGE_NAME ${in_package_name})

    SET(DOCS_TOP_BUILD_DIRECTORY ${${PACKAGE_NAME}_BINARY_DIR})
    IF("${DOCS_TOP_BUILD_DIRECTORY}" STREQUAL "") 
	MESSAGE(FATAL_ERROR "LINTEL_DOCS_CONFIG misuse, ${PACKAGE_NAME} does not find \${${PACKAGE_NAME}_BINARY_DIR}")
    ENDIF("${DOCS_TOP_BUILD_DIRECTORY}" STREQUAL "") 

    OPTION(BUILD_DOXYGEN_DOCUMENTATION "Should we build/install the doxygan documentation" ON)

    IF(BUILD_DOXYGEN_DOCUMENTATION)
        INCLUDE(FindDoxygen)

        IF(DOXYGEN_FOUND)
	   SET(DOXYGEN_DOCUMENTATION_ENABLED ON)
        ELSE(DOXYGEN_FOUND)
           MESSAGE("Could NOT find doxygen program, doxygen documentation will not be built")
           LIST(APPEND LINTEL_FIND_ALL_NOTFOUND docs-${in_package_name})
        ENDIF(DOXYGEN_FOUND)

        FIND_PROGRAM(CMAKE_BINARY cmake)
    ENDIF(BUILD_DOXYGEN_DOCUMENTATION)
ENDMACRO(LINTEL_DOCS_CONFIG)

MACRO(LDB_SETVAR_IFDIR var dir)
    IF(IS_DIRECTORY ${dir})
        SET(${var} ${dir})
    ELSE(IS_DIRECTORY ${dir})
        SET(${var} "")
    ENDIF(IS_DIRECTORY ${dir})
ENDMACRO(LDB_SETVAR_IFDIR)

MACRO(LINTEL_DOCS_BUILD)
    IF(DOXYGEN_DOCUMENTATION_ENABLED)
        LDB_SETVAR_IFDIR(DOXYGEN_DOTFILE_DIRS ${${PACKAGE_NAME}_SOURCE_DIR}/doc/doxygen-figures)
	LDB_SETVAR_IFDIR(DOXYGEN_EXAMPLE_PATH ${${PACKAGE_NAME}_SOURCE_DIR}/src/example)

        FIND_FILE(DOXYGEN_CONFIG_IN
	    	  doxygen.config.in
		  PATHS ${CMAKE_HOME_DIRECTORY}/doc
		        ${CMAKE_INSTALL_PREFIX}/share/Lintel
	   	        /usr/share/Lintel
	)
        CONFIGURE_FILE(${DOXYGEN_CONFIG_IN}
	               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.config @ONLY)
     
        FILE(GLOB_RECURSE DOXYGEN_DEP_INCLUDES1 ${CMAKE_HOME_DIRECTORY}/include/*.hpp)
        FILE(GLOB_RECURSE DOXYGEN_DEP_INCLUDES2 ${CMAKE_CACHEFILE_DIR}/include/*.hpp)
     
	# Create a clean directory for the output or if files are deleted the
	# obsolete man/html pages will still be installed.
        ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html/index.html
		      COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/doxygen.bak
		      COMMAND [ ! -d ${CMAKE_CURRENT_BINARY_DIR}/doxygen ] 
		              || mv ${CMAKE_CURRENT_BINARY_DIR}/doxygen
	                         ${CMAKE_CURRENT_BINARY_DIR}/doxygen.bak
		      COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}/doxygen
     	              COMMAND ${DOXYGEN_EXECUTABLE} 
     		   	      ${CMAKE_CURRENT_BINARY_DIR}/doxygen.config
      		      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/doxygen.config
     		              ${DOXYGEN_DEP_INCLUDES1}
     			      ${DOXYGEN_DEP_INCLUDES2}
			      ${DOXYGEN_DEP_EXTRAS}
                           )
     
        ADD_CUSTOM_TARGET(doc ALL 
     	  	     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html/index.html)
     
        # TODO: figure out how to clean; the following doesn't work.
        # SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES doxygen)
	FIND_FILE(LINTEL_DOCS_CMAKE LintelDocs.cmake ${CMAKE_MODULE_PATH})
	
	IF(NOT LINTEL_DOCS_CMAKE)
            MESSAGE(FATAL_ERROR "Huh, how can we not find LintelDocs.cmake when we are it")
	ENDIF(NOT LINTEL_DOCS_CMAKE)

        INSTALL(CODE "EXEC_PROGRAM(${CMAKE_BINARY} ARGS -DBUILDDOC=${CMAKE_CURRENT_BINARY_DIR} -DTARGET=\${CMAKE_INSTALL_PREFIX} -DPACKAGE=${PACKAGE_NAME} -P ${LINTEL_DOCS_CMAKE})")
    ENDIF(DOXYGEN_DOCUMENTATION_ENABLED)
ENDMACRO(LINTEL_DOCS_BUILD)

# For windows documentation install, see:
# http://www.cmake.org/pipermail/cmake/2006-August/010786.html

# Below causes an actual install of things when the install target of
# LINTEL_DOCS_BUILD executes.
IF(DEFINED BUILDDOC AND DEFINED TARGET AND DEFINED PACKAGE)
    # Script for doing the actual install work...
    IF(NOT IS_DIRECTORY "${BUILDDOC}/doxygen/html")
	MESSAGE(FATAL_ERROR "Missing ${BUILDDOC}/doxygen/html")
    ENDIF(NOT IS_DIRECTORY "${BUILDDOC}/doxygen/html")

    MESSAGE("Install html to ${TARGET}/doc/${PACKAGE}")
    
    # TODO: figure out how to do this without outcall to cp
    # INSTALL(DIRECTORY ${BUILDDOC}/doxygen/html DESTINATION ${TARGET}/doc/${PACKAGE})
    # does not work because install is not scriptable, and we can't have
    # an install target because the files won't exist until after the
    # build is completed.

    FILE(MAKE_DIRECTORY "$ENV{DESTDIR}${TARGET}/share/doc/${PACKAGE}")

    EXEC_PROGRAM(cp ARGS -rp "${BUILDDOC}/doxygen/html" 
	                 "$ENV{DESTDIR}${TARGET}/share/doc/${PACKAGE}")

    MESSAGE("Install man pages to ${TARGET}/share/man/man3")
    FILE(MAKE_DIRECTORY "$ENV{DESTDIR}${TARGET}/share/man/man3")

    EXEC_PROGRAM(cp ARGS -rp "${BUILDDOC}/doxygen/man/man3/*" 
	                 "$ENV{DESTDIR}${TARGET}/share/man/man3")
ENDIF(DEFINED BUILDDOC AND DEFINED TARGET AND DEFINED PACKAGE)

############################################
############### latex
############################################

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

MACRO(LINTEL_LATEX_REQUIRES variable doc_name)
    SET(${variable} ${LINTEL_LATEX_REBUILD_ENABLED})
    IF(LINTEL_LATEX_REBUILD_ENABLED)
        FOREACH(llr_filename ${ARGN})
            EXECUTE_PROCESS(COMMAND kpsewhich ARGS ${llr_filename}
                            RESULT_VARIABLE llr_result
                            OUTPUT_VARIABLE llr_output
                            OUTPUT_STRIP_TRAILING_WHITESPACE)
            IF("${llr_output}" STREQUAL "")
                MESSAGE("Unable to find latex file ${llr_filename} for ${doc_name}")
                SET(${variable} FALSE)
            ELSEIF(NOT EXISTS "${llr_output}")
                MESSAGE("Found ${llr_output} for ${doc_name} but it does not exist?")
                SET(${variable} FALSE)
            ENDIF("${llr_output}" STREQUAL "")
        ENDFOREACH(llr_filename)
    ENDIF(LINTEL_LATEX_REBUILD_ENABLED)
ENDMACRO(LINTEL_LATEX_REQUIRES)            


############################################
############### pod2man
############################################

# LINTEL_POD2MAN will run pod2man on an input file, either in the current source or build
# directory.  it will also install the manual page into
# ${CMAKE_INSTALL_PREFIX}/share/man/man${section} The macro attempts to guess the appropriate
# section type from the section number, but if it is unable to do so then the value can be maually
# specified until LINTEL_POD2MAN is updated.  Similarly, the default output name is guessed
# from the input name but may also be specified.

# LINTEL_POD2MAN(input_file section release [section-type [output-name]])
# The value "-" for section-type means take the default. "" does not seem to work in cmake 2.6; 
# the argument is lost.
#
# LINTEL_POD2MAN(program 1 "Package ${PACKAGE_VERSION}")
# LINTEL_POD2MAN(program 6 "Package ${PACKAGE_VERSION}" "Games/Shooter")
# LINTEL_POD2MAN(program.cpp 1 "Package ${PACAKGE_VERSION}" "-" program.1)

MACRO(LINTEL_POD2MAN_SETUP)
    LINTEL_WITH_PROGRAM(POD2MAN pod2man)
    FIND_PATH(LINTELDOCS_CMAKE LintelDocs.cmake PATHS ${CMAKE_MODULE_PATH} NO_DEFAULT_PATH)
    IF(NOT LINTELDOCS_CMAKE)
        MESSAGE(FATAL_ERROR "In LintelDocs.cmake, but can't find it in ${CMAKE_MODULE_PATH}?")
    ENDIF(NOT LINTELDOCS_CMAKE)
    LINTEL_REQUIRED_PROGRAM(CMAKE cmake) # We're running it, have to be able to find it.
    SET(LINTEL_POD2MAN_ENABLED ${POD2MAN_ENABLED})
ENDMACRO(LINTEL_POD2MAN_SETUP)

MACRO(LINTEL_POD2MAN file section release)
    IF(LINTEL_POD2MAN_ENABLED)
        LINTEL_POD2MAN_ACTUAL(${file} ${section} ${release} ${ARGN})
    ELSEIF(DEFINED LINTEL_POD2MAN_ENABLED)
        MESSAGE("Warning: Missing pod2man (${POD2MAN_ENABLED}), unable to generate manpage for ${file}")
    ELSE(LINTEL_POD2MAN_ENABLED)
        MESSAGE(FATAL_ERROR "ERROR: Missing call to LINTEL_POD2MAN_SETUP() prior to call to LINTEL_POD2MAN")
    ENDIF(LINTEL_POD2MAN_ENABLED)
ENDMACRO(LINTEL_POD2MAN)
 
MACRO(LINTEL_POD2MAN_ACTUAL file section release)
    SET(cmake_bug ${ARGV3}) # ARGV3 is always "not defined"

    IF(NOT DEFINED cmake_bug OR cmake_bug STREQUAL "-")
        IF(0)
        ELSEIF (${section} EQUAL 1) 
            SET(lintel_pod2man_type "User Commands")
        ELSE(0)
            MESSAGE("Warning: Unknown section ${section}, using manual type 'Unknown'")
            SET(lintel_pod2man_type Unknown)
        ENDIF(0)
    ELSE(NOT DEFINED cmake_bug OR cmake_bug STREQUAL "-")
        SET(lintel_pod2man_type ${ARGV3})
    ENDIF(NOT DEFINED cmake_bug OR cmake_bug STREQUAL "-")
    
    IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        SET(lintel_pod2man_input ${CMAKE_CURRENT_SOURCE_DIR}/${file})
    ELSE(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        SET(lintel_pod2man_input ${CMAKE_CURRENT_BINARY_DIR}/${file})
    ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file})

    SET(cmake_bug ${ARGV4})
    IF(DEFINED cmake_bug)
        SET(lintel_pod2man_output ${cmake_bug})
    ELSE(DEFINED cmake_bug)
        SET(lintel_pod2man_output ${file}.${section})
    ENDIF(DEFINED cmake_bug)

    ADD_CUSTOM_TARGET(${lintel_pod2man_output} ALL
                      COMMAND ${CMAKE_PATH} -DPOD2MAN_PATH=${POD2MAN_PATH}
                              -D LPA_IN=${lintel_pod2man_input} -D LPA_OUT=${lintel_pod2man_output}
                              -D LPA_RELEASE=${release} -D LPA_TYPE=${lintel_pod2man_type}
                              -P ${LINTELDOCS_CMAKE}/LintelDocs.cmake
                      VERBATIM)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${lintel_pod2man_output}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man${section})
ENDMACRO(LINTEL_POD2MAN_ACTUAL)

# Seems like there ought to be a way to call the command and redirect the output in the
# custom target above, but at least in cmake 2.6 I can't figure out how.
IF(0)
ELSEIF(DEFINED POD2MAN_PATH AND DEFINED LPA_IN AND DEFINED LPA_OUT AND DEFINED LPA_RELEASE 
       AND DEFINED LPA_TYPE)
    FILE(REMOVE ${LPA_OUT})
    EXECUTE_PROCESS(COMMAND ${POD2MAN_PATH} --release=${LPA_RELEASE} --center=${LPA_TYPE} ${LPA_IN}
                    OUTPUT_FILE ${LPA_OUT} RESULT_VARIABLE LPA_SUCCESS)
    IF(NOT ${LPA_SUCCESS} STREQUAL "0") 
        MESSAGE(FATAL_ERROR "ERROR: Unable to create ${LPA_OUT}. Command ${POD2MAN_PATH} --release=${LPA_RELEASE} --center=${LPA_TYPE} ${LPA_IN} failed: ${LPA_SUCCESS}")
    ENDIF(NOT ${LPA_SUCCESS} STREQUAL "0") 
ENDIF(0)


