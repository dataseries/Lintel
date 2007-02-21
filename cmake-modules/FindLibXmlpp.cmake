# derived from http://websvn.kde.org/trunk/KDE/kdelibs/cmake/modules/
# - Try to find LibXml++
# Once done this will define
#
#  LIBXMLPP_FOUND - system has LibXml++
#  LIBXMLPP_INCLUDE_DIR - the LibXml++ include directory
#  LIBXMLPP_LIBRARIES - the libraries needed to use LibXml++
#  LIBXMLPP_DEFINITIONS - Compiler switches required for using LibXml++
# (c) Hewlett Packard Development Company, 2006
#
# Copyright (c) 2006, Alexander Neundorf <neundorf@kde.org>
# This code is available under the BSD license, see licenses/BSD for details.

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (LIBXMLPP_INCLUDE_DIR AND LIBXMLPP_LIBRARIES)
   # in cache already
   SET(LibXmlpp_FIND_QUIETLY TRUE)
ENDIF (LIBXMLPP_INCLUDE_DIR AND LIBXMLPP_LIBRARIES)

IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   INCLUDE(UsePkgConfig)
   PKGCONFIG(libxml++-2.6 _LibXmlppIncDir _LibXmlppLinkDir _LibXmlppLinkFlags _LibXmlppCflags)
   SET(LIBXMLPP_DEFINITIONS ${_LibXmlppCflags})
ENDIF (NOT WIN32)

FIND_PATH(LIBXMLPP_INCLUDE_DIR libxml++/libxml++.h
   PATHS
   ${_LibXmlppIncDir}
   PATH_SUFFIXES libxml++-2.6
   )

FIND_LIBRARY(LIBXMLPP_LIBRARIES NAMES xml++-2.6
   PATHS
   ${_LibXmlppLinkDir}
   )

IF (LIBXMLPP_INCLUDE_DIR AND LIBXMLPP_LIBRARIES)
   SET(LIBXMLPP_FOUND TRUE)
ELSE (LIBXMLPP_INCLUDE_DIR AND LIBXMLPP_LIBRARIES)
   SET(LIBXMLPP_FOUND FALSE)
ENDIF (LIBXMLPP_INCLUDE_DIR AND LIBXMLPP_LIBRARIES)

IF (LIBXMLPP_FOUND)
   IF (NOT LibXmlpp_FIND_QUIETLY)
      MESSAGE(STATUS "Found LibXml++: ${LIBXMLPP_LIBRARIES}")
   ENDIF (NOT LibXmlpp_FIND_QUIETLY)
ELSE (LIBXMLPP_FOUND)
   IF (LibXmlpp_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could NOT find LibXml++")
   ENDIF (LibXmlpp_FIND_REQUIRED)
ENDIF (LIBXMLPP_FOUND)

MARK_AS_ADVANCED(LIBXMLPP_INCLUDE_DIR LIBXMLPP_LIBRARIES)

