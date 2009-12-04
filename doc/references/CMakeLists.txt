#
# (c) Copyright 2009, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake description for the doc/references directory

# Unfortunately, FILE(GLOB *bib) doesn't do what we want, if a new
# .bib file is added, the file isn't automatically regenerated.
SET(BIB_FILES 
    aadebug.bib
    acmtocs.bib
    acmtoplas.bib
    asplos.bib
    books.bib
    cacm.bib
    csur.bib
    datamation.bib
    disc.bib
    distributedcomputing.bib
    dsn.bib
    fast.bib
    efts.bib
    hotdep.bib
    hotos.bib
    icdcs.bib
    icpp.bib
    ics.bib
    ieee-computer.bib
    ieeejsac.bib
    ieee-icin.bib
    ieeetoc.bib
    ieeetpds.bib
    isca.bib
		jacm.bib
    journal-high-perf-compute-apps.bib
    mascots.bib
    middleware.bib
    nsdi.bib
    osdi.bib
    os-review.bib
    pldi.bib
    scientific-programming.bib
    siam-parallel-scientific.bib
    sigact.bib
    sigcomm.bib
    sigmod.bib
    sosp.bib
    sss.bib
    supercomputing-review.bib
    tcsc.bib
    toit.bib
    tr.bib
    tse.bib
    usenix.bib
    vldb.bib
    web-sites.bib
    wmcsa.bib
    wmrd.bib
)

ADD_CUSTOM_TARGET(all-bib ALL 
                  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build-bibs.sh 
		          ${CMAKE_CURRENT_SOURCE_DIR} ${BIB_FILES}
		  DEPENDS ${BIB_FILES})

INSTALL(FILES ${BIB_FILES} ${CMAKE_CURRENT_BINARY_DIR}/lintel-latex-all.bib 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/lintel-latex-rebuild)

ADD_TEST(web-sites-check ${CMAKE_CURRENT_SOURCE_DIR}/check-web-sites-bib.sh)
