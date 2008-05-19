#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Main cmake control file

# ELSEIF came with 2.4.4
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.4 FATAL_ERROR)

PROJECT(Lintel)

SET(LINTEL_VERSION 0.0)
SET(LINTEL_ABI_VERSION 0)

INCLUDE("CMakeConfig.txt")
INCLUDE("cmake-modules/LintelCMakeUtil.cmake")

ENABLE_TESTING()

ADD_SUBDIRECTORY(include/Lintel)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(doc)
ADD_SUBDIRECTORY(cmake-modules)

INSTALL(FILES com_hp_hpl_lintel.m4
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/aclocal)
