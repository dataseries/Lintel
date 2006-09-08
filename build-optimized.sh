#!/bin/sh
###############################################################################
#
# File:         build-optimized.sh
# RCS:          $Header$
# Description:  Typical sequence of build steps
#		Do this in Lintel first, then in each of the directories
#		that depend on it.
# Author:       Eric Anderson
# Created:      Thu Mar 16 18:07:47 2006
# Modified:     Thu Mar 16 18:10:15 2006 (john wilkes) john.wilkes@hp.com
# Language:     Shell-script
# Package:      N/A
# Status:       Experimental (Do Not Distribute)
#
# (C) Copyright 2006, Hewlett-Packard Company, all rights reserved.
#
###############################################################################
set -e				# stop on error
make distclean || true		# ok if fails - eg, if configure hasn't run

				# rebuild Makefile, etc
./configure --enable-optmode=optimize --prefix=/home/`whoami`/build/optimized

make all			# rebuild everything
make check			# run internal regression tests
make install			# copy over to target directory (from --prefix)
