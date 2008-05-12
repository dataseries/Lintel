#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# All of the options and dependencies for the various cmake sub-bits
#
# Find the Lintel, LintelPThread includes and library
#
#  set LINTEL_FIND_REQUIRED to require the LINTEL library
#  otherwise, the user will be given a WITH_LINTEL option.
#
#  set LINTELPTHREAD_FIND_REQUIRED to require the LINTELPTHREAD library
#  otherwise, the user will be given a WITH_LINTELPTHREAD option.
#
#  LINTEL_INCLUDE_DIR - where to find openssl/sha.h
#  LINTEL_LIBRARIES   - List of libraries when using LINTEL
#  LINTEL_ENABLED     - True if LINTEL is enabled
# 
#  LINTELPTHREAD_INCLUDE_DIR - where to find openssl/sha.h
#  LINTELPTHREAD_LIBRARIES   - List of libraries when using LINTELPTHREAD
#  LINTELPTHREAD_ENABLED     - True if LINTELPTHREAD is enabled

INCLUDE(LintelFindLibrary)

LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE(LINTEL Lintel/LintelAssert.hpp Lintel)
LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE(LINTELPTHREAD Lintel/PThread.hpp LintelPThread)
