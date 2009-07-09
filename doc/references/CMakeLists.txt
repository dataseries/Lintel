#
# (c) Copyright 2009, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake description for the doc/references directory

# Unfortunately, FILE(GLOB *bib) doesn't do what we want, if a new
# .bib file is added, the file isn't automatically regenerated.
SET(BIB_FILES 
    acmtocs.bib
    acmtoplas.bib
    asplos.bib
    books.bib
    cacm.bib
    csur.bib
    datamation.bib
    disc.bib
    distributedcomputing.bib
    hotdep.bib
    hotos.bib
    ics.bib
    ieee-computer.bib
    ieeetoc.bib
    isca.bib
    journal-high-perf-compute-apps.bib
    mascots.bib
    nsdi.bib
    osdi.bib
    os-review.bib
    scientific-programming.bib
    siam-parallel-scientific.bib
    sigact.bib
    sigcomm.bib
    sigmod.bib
    sosp.bib
    supercomputing-review.bib
    vldb.bib
    web-sites.bib
)

ADD_CUSTOM_TARGET(all-bib ALL 
                  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build-bibs.sh 
		          ${CMAKE_CURRENT_SOURCE_DIR} ${BIB_FILES}
		  DEPENDS ${BIB_FILES})

INSTALL(FILES ${BIB_FILES} ${CMAKE_CURRENT_BINARY_DIR}/lintel-latex-all.bib 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/lintel-latex-rebuild)

ADD_TEST(web-sites-check ${CMAKE_CURRENT_SOURCE_DIR}/check-web-sites-bib.sh)
