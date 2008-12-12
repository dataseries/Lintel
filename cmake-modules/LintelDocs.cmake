#
# (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Macros for dealing with doxygen.

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
#   - DOXYGEN_EXTRA_DOTFILE_DIRS: additional .dot file directories
#     Default is project-root/doc/doxygen-figures

# If you need to customize the doxygen config, copy
# .../share/Lintel/doxygen.config.in to docs/doxygen.config.in, and
# edit as appropriate.  Otherwise the standard input config will be
# used from the Lintel project.

MACRO(LINTEL_DOCS_CONFIG in_package_name)
    SET(PACKAGE_NAME ${in_package_name})

    SET(DOCS_TOP_BUILD_DIRECTORY ${${PACKAGE_NAME}_BINARY_DIR})
    IF("${DOCS_TOP_BUILD_DIRECTORY}" STREQUAL "") 
	MESSAGE(FATAL_ERROR "LINTEL_DOCS_CONFIG misuse, ${PACKAGE_NAME} does not find \${${PACKAGE_NAME}_BINARY_DIR}")
    ENDIF("${DOCS_TOP_BUILD_DIRECTORY}" STREQUAL "") 

    OPTION(BUILD_DOCUMENTATION "Should we build/install the documentation" ON)

    IF(BUILD_DOCUMENTATION)
        INCLUDE(FindDoxygen)

        IF(DOXYGEN_FOUND)
	   SET(DOCUMENTATION_ENABLED ON)
        ELSE(DOXYGEN_FOUND)
           MESSAGE("WARNING: doxygen not found, unable to build documentation")
        ENDIF(DOXYGEN_FOUND)

        FIND_PROGRAM(CMAKE_BINARY cmake)
    ENDIF(BUILD_DOCUMENTATION)
ENDMACRO(LINTEL_DOCS_CONFIG)

MACRO(LINTEL_DOCS_BUILD)
    IF(DOCUMENTATION_ENABLED)
	IF(NOT DEFINED DOXYGEN_EXAMPLE_PATH)
	    SET(DOXYGEN_EXAMPLE_PATH ${${PACKAGE_NAME}_SOURCE_DIR}/src/example)
	ENDIF(NOT DEFINED DOXYGEN_EXAMPLE_PATH)

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
     
        ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html/index.html
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
    ENDIF(DOCUMENTATION_ENABLED)
ENDMACRO(LINTEL_DOCS_BUILD)

# For windows documentation install, see:
# http://www.cmake.org/pipermail/cmake/2006-August/010786.html

IF(BUILDDOC AND TARGET AND PACKAGE)
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
ENDIF(BUILDDOC AND TARGET AND PACKAGE)

