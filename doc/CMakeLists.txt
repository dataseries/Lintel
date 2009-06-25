#
# (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake description for the doc directory

INCLUDE(LintelDocs)

LINTEL_DOCS_BUILD()

INSTALL(FILES doxygen.config.in 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/Lintel)

ADD_SUBDIRECTORY(references)
