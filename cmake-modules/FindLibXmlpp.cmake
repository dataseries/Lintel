# derived from http://websvn.kde.org/trunk/KDE/kdelibs/cmake/modules/
# - Try to find LibXml++
# Once done this will define
#
#  LIBXMLPP_FOUND - system has LibXml++
#  LIBXMLPP_INCLUDE_DIR - the LibXml++ include directory
#  LIBXMLPP_INCLUDES - all includes for LibXml++ include
#  LIBXMLPP_LIBRARIES - the libraries needed to use LibXml++
#  LIBXMLPP_DEFINITIONS - Compiler switches required for using LibXml++
# (c) Hewlett Packard Development Company, 2006
#
# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the below copyright.

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

SET(LIBXMLPP_INCLUDES ${LIBXMLPP_INCLUDE_DIR} ${LIBXMLPP_EXTRA_INCLUDES})

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

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products 
#    derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
