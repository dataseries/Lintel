#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Macro for finding headers, libraries and the combination; also does
# all of the WITH_xxx support so users can disable libraries that they
# have installed but don't want to use.
#
# LINTEL_FIND_HEADER sets ${variable}_INCLUDE_DIR.  It also
#   sets up an option WITH_${variable} unless ${variable}_FIND_REQUIRED 
#   is on.  It sets ${variable}_ENABLED based on finding the header
#   and WITH_${variable}

# LINTEL_FIND_LIBRARY calls LINTEL_FIND_HEADER, and then sets
#   ${variable}_LIBRARIES, and ${variable}_ENABLED based on the
#   value from LINTEL_FIND_HEADER and whether the library was found.  
#   If you really need to know if it is found ${variable}_FOUND is also set.

MACRO(LINTEL_FIND_HEADER variable header)
    IF(${variable}_INCLUDE_DIR)
        # Already in cache, be silent
        SET(${variable}_FIND_QUIETLY YES)
    ENDIF(${variable}_INCLUDE_DIR)
    
    # This is the recommended cmake idiom to use a locally built version
    # of a library in preference to the system one.

    FIND_PATH(${variable}_INCLUDE_DIR ${header}
        ${CMAKE_INSTALL_PREFIX}/include
        NO_DEFAULT_PATH
    )

    FIND_PATH(${variable}_INCLUDE_DIR ${header})

    MARK_AS_ADVANCED(${variable}_INCLUDE_DIR)

    IF(${variable}_FIND_REQUIRED)
        IF(NOT ${variable}_INCLUDE_DIR)
            MESSAGE(STATUS "Looked for header file ${header} in ${CMAKE_INSTALL_PREFIX}/include and system paths")
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
ENDMACRO(LINTEL_FIND_HEADER)

MACRO(LINTEL_WITH_HEADER variable header)
    LINTEL_FIND_HEADER(${variable} ${header})

    IF(${variable}_INCLUDE_DIR)
	SET(WITH_${variable}_DEFAULT ON)
    ELSE(${variable}_INCLUDE_DIR)
	SET(WITH_${variable}_DEFAULT OFF)
    ENDIF(${variable}_INCLUDE_DIR)

    SET(WITH_${variable} ${WITH_${variable}_DEFAULT} CACHE BOOL "Enable compilation depending on header file ${header}")
    IF(WITH_${variable} AND ${variable}_INCLUDE_DIR)
	SET(${variable}_ENABLED ON)
    ELSE(WITH_${variable} AND ${variable}_INCLUDE_DIR)
        SET(${variable}_ENABLED OFF)
    ENDIF(WITH_${variable} AND ${variable}_INCLUDE_DIR)
ENDMACRO(LINTEL_WITH_HEADER)  

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
        SET(${variable}_LIBRARIES ${${variable}_LIBRARY})
    ELSE (${variable}_INCLUDE_DIR AND ${variable}_LIBRARY)
        SET(${variable}_FOUND FALSE)
        SET(${variable}_LIBRARIES)
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

    IF(${variable}_FOUND)
	SET(WITH_${variable}_DEFAULT ON)
    ELSE(${variable}_FOUND)
	SET(WITH_${variable}_DEFAULT OFF)
    ENDIF(${variable}_FOUND)

    SET(WITH_${variable} ${WITH_${variable}_DEFAULT} CACHE BOOL "Enable compilation depending on library ${libname}")
    IF(WITH_${variable} AND ${variable}_FOUND)
	SET(${variable}_ENABLED ON)
    ELSE(WITH_${variable} AND ${variable}_FOUND)
        SET(${variable}_ENABLED OFF)
    ENDIF(WITH_${variable} AND ${variable}_FOUND)
ENDMACRO(LINTEL_WITH_LIBRARY)  

# Find the ${Variable} includes and library
#
#  set ${VARIABLE}_FIND_REQUIRED to require the ${VARIABLE} library
#  otherwise, the user will be given a WITH_${VARIABLE} option.
# 
#  ${VARIABLE}_INCLUDE_DIR - where to find openssl/sha.h
#  ${VARIABLE}_LIBRARIES   - List of libraries when using ${VARIABLE}
#  ${VARIABLE}_ENABLED     - True if ${VARIABLE} is enabled

MACRO(LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE variable header libname)
    IF(${variable}_FIND_REQUIRED)
	LINTEL_FIND_LIBRARY(${variable} ${header} ${libname})
    ELSE(${variable}_FIND_REQUIRED)
	LINTEL_WITH_LIBRARY(${variable} ${header} ${libname})
    ENDIF(${variable}_FIND_REQUIRED)
ENDMACRO(LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE)
 
