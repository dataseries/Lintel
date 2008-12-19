#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake rules for the src/BatchParallel directory

SET(BATCH_PARALLEL_FILES
	checksum.pm
	common.pm
	compress.pm
	jobsfile.pm
	make.pm
	tar.pm
)

IF(PERL_ENABLED)
    INSTALL(FILES ${BATCH_PARALLEL_FILES}
    	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/bp_modules/BatchParallel)
ENDIF(PERL_ENABLED)