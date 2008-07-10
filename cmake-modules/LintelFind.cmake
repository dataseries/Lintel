#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details

# Macro for finding binaries, headers, and libraries; also does all of
# the WITH_xxx support so users can disable libraries that they have
# installed but don't want to use.  Normally you would use the
# LINTEL_WITH_* or LINTEL_REQUIRED_* variants of the macros.

# LINTEL_FIND_HEADER(variable header) sets ${variable}_INCLUDE_DIR,
#   ${variable}_INCLUDES, and ${variable}_ENABLED.
#
#   The header must exist if ${variable}_FIND_REQUIRED is set.
#   It also sets up an option WITH_${variable} unless
#   ${variable}_FIND_REQUIRED is on.  It sets ${variable}_ENABLED based
#   on finding the header and WITH_${variable}; you can set 
#   ${variable}_EXTRA_INCLUDE_PATHS to specify additional locations it 
#   should look, and ${variable}_EXTRA_INCLUDES to specify extra directories 
#   to put in the ${variable}_INCLUDES variable.

# LINTEL_FIND_LIBRARY(variable header libname) sets ${variable}_LIBRARY,
#   ${variable}_LIBRARIES, and ${variable}_ENABLED after calling 
#   LINTEL_FIND_HEADER to find the header.
#
#   The library must exist if ${variable}_FIND_REQUIRED is set.  You can set
#   ${variable}_EXTRA_LIBRARIES to specify extra librareis to put in the
#   ${variable}_LIBRARIES variable.

# LINTEL_FIND_PROGRAM(variable program) sets ${variable}_PATH  The program
#   must exist if ${variable}_FIND_REQUIRED is set.

# LINTEL_WITH_*(...) adds an option WITH_${variable}, and sets
#   ${variable}_ENABLED if the thing was found and we should use it.  If
#   we should use it and it was not found, a message is printed out
#   explaining what was not found.  If ${variable}_MISSING_EXTRA is set,
#   that string is also printed.

# LINTEL_REQUIRED_*(...) tries to find the thing and produces and
#   error message if it is not found.  If ${variable}_MISSING_EXTRA is
#   set, that string is also printed.

# TODO: there is a bunch of duplicate code in the below macros,
# eliminate it.

### headers

MACRO(LINTEL_FIND_HEADER variable header)
    IF(${variable}_INCLUDE_DIR)
        # Already in cache, be silent
        SET(${variable}_FIND_QUIETLY YES)
    ENDIF(${variable}_INCLUDE_DIR)
    
    # This is the recommended cmake idiom to use a locally built version
    # of a header in preference to the system one.

    FIND_PATH(${variable}_INCLUDE_DIR ${header}
        ${CMAKE_INSTALL_PREFIX}/include
        NO_DEFAULT_PATH
    )

    FIND_PATH(${variable}_INCLUDE_DIR ${header})

    IF(${variable}_EXTRA_INCLUDE_PATHS)
	FIND_PATH(${variable}_INCLUDE_DIR ${header}
	          PATHS ${${variable}_EXTRA_INCLUDE_PATHS})
    ENDIF(${variable}_EXTRA_INCLUDE_PATHS)

    MARK_AS_ADVANCED(${variable}_INCLUDE_DIR)

    IF(${variable}_FIND_REQUIRED)
        IF(NOT ${variable}_INCLUDE_DIR)
            MESSAGE(STATUS "Looked for header file ${header} in ${CMAKE_INSTALL_PREFIX}/include, system paths, and '${${variable}_EXTRA_INCLUDE_PATHS}' (if set)")
	    IF(DEFINED ${variable}_MISSING_EXTRA)
	        MESSAGE(STATUS "${${variable}_MISSING_EXTRA")
	    ENDIF(DEFINED ${variable}_MISSING_EXTRA)
            MESSAGE(FATAL_ERROR "ERROR: Could NOT find header file ${header}")
        ENDIF(NOT ${variable}_INCLUDE_DIR)
	SET(${variable}_ENABLED ON)
    ENDIF(${variable}_FIND_REQUIRED)

    IF(${variable}_INCLUDE_DIR)
        IF(NOT ${variable}_FIND_QUIETLY)
            MESSAGE(STATUS "Found header ${header} in ${${variable}_INCLUDE_DIR}")
        ENDIF(NOT ${variable}_FIND_QUIETLY)
    ELSE(${variable}_INCLUDE_DIR)
        SET(${variable}_INCLUDE_DIR "")
    ENDIF(${variable}_INCLUDE_DIR)

    SET(${variable}_INCLUDES ${${variable}_INCLUDE_DIR}
	${${variable}_EXTRA_INCLUDES})
ENDMACRO(LINTEL_FIND_HEADER)

MACRO(LINTEL_WITH_HEADER variable header)
    LINTEL_FIND_HEADER(${variable} ${header})

    SET(WITH_${variable} ON CACHE BOOL "Enable compilation depending on header file ${header}")
    IF(WITH_${variable} AND ${variable}_INCLUDE_DIR)
	SET(${variable}_ENABLED ON)
    ELSE(WITH_${variable} AND ${variable}_INCLUDE_DIR)
        SET(${variable}_ENABLED OFF)
    ENDIF(WITH_${variable} AND ${variable}_INCLUDE_DIR)

    IF(WITH_${variable} AND NOT ${variable}_ENABLED)
        MESSAGE(STATUS "WITH_${variable} on, but could not find header file ${header}")
        IF(DEFINED ${variable}_MISSING_EXTRA)
	    MESSAGE(STATUS "${${variable}_MISSING_EXTRA}")
        ENDIF(DEFINED ${variable}_MISSING_EXTRA)
    ENDIF(WITH_${variable} AND NOT ${variable}_ENABLED)
ENDMACRO(LINTEL_WITH_HEADER)  

MACRO(LINTEL_REQUIRED_HEADER variable header)
    SET(${variable}_FIND_REQUIRED ON)
    LINTEL_FIND_HEADER(${variable} ${header})
ENDMACRO(LINTEL_REQUIRED_HEADER variable header)

### Libraries

# TODO: consider a check that the library and header share most of their
# prefix and generate a warning if they don't.

MACRO(LINTEL_FIND_LIBRARY variable header libname)
    LINTEL_FIND_HEADER(${variable} ${header})

    FIND_LIBRARY(${variable}_LIBRARY 
        NAMES ${libname} 
        PATHS ${CMAKE_INSTALL_PREFIX}/lib
        NO_DEFAULT_PATH)

    FIND_LIBRARY(${variable}_LIBRARY NAMES ${libname})

    MARK_AS_ADVANCED(${variable}_LIBRARY)

    IF (${variable}_INCLUDE_DIR AND ${variable}_LIBRARY)
        SET(${variable}_FOUND TRUE)
        SET(${variable}_LIBRARIES ${${variable}_LIBRARY} 
	    ${${variable}_EXTRA_LIBRARIES})
    ELSE (${variable}_INCLUDE_DIR AND ${variable}_LIBRARY)
        SET(${variable}_FOUND FALSE)
        SET(${variable}_LIBRARIES ${variable}-NOTFOUND)
    ENDIF (${variable}_INCLUDE_DIR AND ${variable}_LIBRARY)

    IF (${variable}_FIND_REQUIRED)
        IF(NOT ${variable}_FOUND)
            MESSAGE(STATUS "Looked for library named ${libname} in ${CMAKE_INSTALL_PREFIX}/lib and system paths")
            MESSAGE(STATUS "got: ${variable}_INCLUDE_DIR=${${variable}_INCLUDE_DIR}")
            MESSAGE(STATUS "got: ${variable}_LIBRARY=${${variable}_LIBRARY}")
            MESSAGE(FATAL_ERROR "ERROR: Could NOT find ${libname} library")
        ENDIF(NOT ${variable}_FOUND)
    ENDIF (${variable}_FIND_REQUIRED)

    IF(NOT ${variable}_FIND_QUIETLY)
	IF(${variable}_FOUND)
            MESSAGE(STATUS "Found library ${libname} as ${${variable}_LIBRARY}")
	ENDIF(${variable}_FOUND)
    ENDIF(NOT ${variable}_FIND_QUIETLY)
ENDMACRO(LINTEL_FIND_LIBRARY)

MACRO(LINTEL_WITH_LIBRARY variable header libname)
    LINTEL_FIND_LIBRARY(${variable} ${header} ${libname})

    SET(WITH_${variable} ON CACHE BOOL "Enable compilation depending on library ${libname}")
    IF(WITH_${variable} AND ${variable}_FOUND)
	SET(${variable}_ENABLED ON)
    ELSE(WITH_${variable} AND ${variable}_FOUND)
        SET(${variable}_ENABLED OFF)
    ENDIF(WITH_${variable} AND ${variable}_FOUND)

    IF(WITH_${variable} AND NOT ${variable}_ENABLED)
        MESSAGE(STATUS "WITH_${variable} on, but could not find header file ${header} or library ${libname}")
        IF(DEFINED ${variable}_MISSING_EXTRA)
	    MESSAGE(STATUS "${${variable}_MISSING_EXTRA}")
        ENDIF(DEFINED ${variable}_MISSING_EXTRA)
    ENDIF(WITH_${variable} AND NOT ${variable}_ENABLED)
ENDMACRO(LINTEL_WITH_LIBRARY)  

MACRO(LINTEL_REQUIRED_LIBRARY variable header libname)
    SET(${variable}_FIND_REQUIRED ON)
    LINTEL_FIND_LIBRARY(${variable} ${header} ${libname})
ENDMACRO(LINTEL_REQUIRED_LIBRARY variable header libname)

### Programs

MACRO(LINTEL_FIND_PROGRAM variable program)
    IF(${variable}_PATH)
	IF(NOT EXISTS "${${variable}_PATH}")
	    MESSAGE(STATUS "WARNING: ${${variable}_PATH} vanished.")
	    SET(${variable}_PATH ${variable}-NOTFOUND)
	ELSE(NOT EXISTS "${${variable}_PATH}")
	    # Already in cache, be silent
	    SET(${variable}_FIND_QUIETLY YES)
	ENDIF(NOT EXISTS "${${variable}_PATH}")
    ENDIF(${variable}_PATH)

    FIND_PROGRAM(${variable}_PATH ${program}
  		 PATHS ${CMAKE_INSTALL_PREFIX}/bin
		 NO_DEFAULT_PATH)

    FIND_PROGRAM(${variable}_PATH ${program})

    MARK_AS_ADVANCED(${variable}_PATH)

    IF(${variable}_FIND_REQUIRED)
        IF(NOT ${variable}_PATH)
            MESSAGE(STATUS "Looked for program ${program} in ${CMAKE_INSTALL_PREFIX}/bin and system paths")
	    IF(DEFINED ${variable}_MISSING_EXTRA)
	        MESSAGE(STATUS "${${variable}_MISSING_EXTRA")
	    ENDIF(DEFINED ${variable}_MISSING_EXTRA)
	    MESSAGE(FATAL_ERROR "ERROR: Could NOT find program ${program}")
        ENDIF(NOT ${variable}_PATH)
    ENDIF(${variable}_FIND_REQUIRED)

    IF(${variable}_PATH)
        SET(${variable}_FOUND ON)
        IF(NOT ${variable}_FIND_QUIETLY)
            MESSAGE(STATUS "Found program ${program} as ${${variable}_PATH}")
        ENDIF(NOT ${variable}_FIND_QUIETLY)
    ENDIF(${variable}_PATH)
ENDMACRO(LINTEL_FIND_PROGRAM)

MACRO(LINTEL_WITH_PROGRAM variable program)
    LINTEL_FIND_PROGRAM(${variable} ${program})

    SET(WITH_${variable} ON CACHE BOOL "Enable compilation using program ${program}")
    IF(WITH_${variable} AND ${variable}_FOUND)
	SET(${variable}_ENABLED ON)
    ELSE(WITH_${variable} AND ${variable}_FOUND)
        SET(${variable}_ENABLED OFF)
    ENDIF(WITH_${variable} AND ${variable}_FOUND)

    IF(WITH_${variable} AND NOT ${variable}_ENABLED)
        MESSAGE(STATUS "WITH_${variable} on, but could not find program ${program}")
        IF(DEFINED ${variable}_MISSING_EXTRA)
	    MESSAGE(STATUS "${${variable}_MISSING_EXTRA}")
        ENDIF(DEFINED ${variable}_MISSING_EXTRA)
    ENDIF(WITH_${variable} AND NOT ${variable}_ENABLED)
ENDMACRO(LINTEL_WITH_PROGRAM)  

MACRO(LINTEL_REQUIRED_PROGRAM variable program)
    SET(${variable}_FIND_REQUIRED ON)
    LINTEL_FIND_PROGRAM(${variable} ${program})
ENDMACRO(LINTEL_REQUIRED_PROGRAM variable program)

# Find the Variable includes and library
#
#  set VARIABLE_FIND_REQUIRED to require the VARIABLE library
#  otherwise, the user will be given a WITH_VARIABLE option.
# 
#  VARIABLE_INCLUDE_DIR - where to find header file for Variable
#  VARIABLE_INCLUDES    - all includes needed for Variable
#  VARIABLE_LIBRARY     - where to find the library for Variable
#  VARIABLE_LIBRARIES   - all libraries needed for Variable
#  VARIABLE_ENABLED     - true if VARIABLE is enabled

# If you are creating FindVariable.cmake, then you want to put the
# documentation from above in, substitute for variable as appropriate,
# and set the variables VARIABLE_EXTRA_INCLUDES and
# VARIABLE_EXTRA_LIBRARIES before calling the
# LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE macro to specify the extra
# includes/libraries needed.

MACRO(LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE variable header libname)
    IF(${variable}_FIND_REQUIRED)
	LINTEL_FIND_LIBRARY(${variable} ${header} ${libname})
    ELSE(${variable}_FIND_REQUIRED)
	LINTEL_WITH_LIBRARY(${variable} ${header} ${libname})
    ENDIF(${variable}_FIND_REQUIRED)
ENDMACRO(LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE)
 
