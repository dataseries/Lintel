#!/bin/sh
#
# (c) Copyright 2009, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# shiny script because I can't figure out how to just do this in cmake.
set -e
cat "$@" >lintel-latex-all.bib
