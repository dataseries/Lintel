#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Main cmake control file

PROJECT(Lintel)

SET(LINTEL_VERSION 0.0)
SET(LINTEL_ABI_VERSION 0)

INCLUDE("CMakeConfig.txt")
INCLUDE("cmake-modules/LintelCMakeUtil.cmake")

ENABLE_TESTING()

ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(doc)
ADD_SUBDIRECTORY(cmake-modules)

INSTALL(FILES com_hp_hpl_lintel.m4
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/aclocal)
