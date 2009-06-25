#
# (c) Copyright 2009, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake description for the doc/references directory

FILE(GLOB BIB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.bib)

ADD_CUSTOM_TARGET(all-bib ALL 
                  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build-bibs.sh ${BIB_FILES}
		  DEPENDS ${BIB_FILES})

INSTALL(FILES ${BIB_FILES} ${CMAKE_CURRENT_BINARY_DIR}/lintel-latex-all.bib 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/lintel-latex-rebuild)

