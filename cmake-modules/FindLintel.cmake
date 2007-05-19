# Find the Lintel includes and library
#
#  LINTEL_INCLUDE_DIR - where to find Lintel/LintelAssert.H, ...
#  LINTEL_LIBRARIES   - List of libraries when using Lintel
#  LINTEL_FOUND       - True if Lintel found.

IF (LINTEL_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LINTEL_FIND_QUIETLY TRUE)
ENDIF (LINTEL_INCLUDE_DIR)

# This is the recommended cmake idiom to use a locally built version
# of a library in preference to the system one.

FIND_PATH(LINTEL_INCLUDE_DIR Lintel/LintelAssert.H
  ${CMAKE_INSTALL_PREFIX}/include
  NO_DEFAULT_PATH
)

FIND_PATH(LINTEL_INCLUDE_DIR Lintel/LintelAssert.H)

# TODO: figure out why in some cases this complains that libLintel.so
# can't have an absolute path, and in others it has no problem.

FIND_LIBRARY(LINTEL_LIBRARY 
  NAMES Lintel 
  PATHS ${CMAKE_INSTALL_PREFIX}/lib
  NO_DEFAULT_PATH)

FIND_LIBRARY(LINTEL_LIBRARY NAMES Lintel)

IF (LINTEL_INCLUDE_DIR AND LINTEL_LIBRARY)
   SET(LINTEL_FOUND TRUE)
   SET(LINTEL_LIBRARIES ${LINTEL_LIBRARY})
ELSE (LINTEL_INCLUDE_DIR AND LINTEL_LIBRARY)
   SET(LINTEL_FOUND FALSE)
   SET(LINTEL_LIBRARIES)
ENDIF (LINTEL_INCLUDE_DIR AND LINTEL_LIBRARY)

IF (LINTEL_FOUND)
   IF (NOT LINTEL_FIND_QUIETLY)
      MESSAGE(STATUS "Found Lintel: ${LINTEL_LIBRARY}")
   ENDIF (NOT LINTEL_FIND_QUIETLY)
ELSE (LINTEL_FOUND)
   IF (LINTEL_FIND_REQUIRED)
      MESSAGE(STATUS "Looked for Lintel libraries named Lintel in ${CMAKE_INSTALL_PREFIX}/lib and system paths")
      MESSAGE(STATUS "got: LINTEL_INCLUDE_DIR=${LINTEL_INCLUDE_DIR}")
      MESSAGE(STATUS "got: LINTEL_LIBRARY=${LINTEL_LIBRARY}")
      MESSAGE(FATAL_ERROR "ERROR: Could NOT find Lintel library")
   ENDIF (LINTEL_FIND_REQUIRED)
ENDIF (LINTEL_FOUND)

MARK_AS_ADVANCED(
  LINTEL_LIBRARY
  LINTEL_INCLUDE_DIR
)
