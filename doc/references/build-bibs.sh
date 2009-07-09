#!/bin/sh
#
# (c) Copyright 2009, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# shiny script because I can't figure out how to just do this in cmake.
set -e
DIR="$1"
shift
if [ ! -d "$DIR" ]; then
    echo "$DIR not a directory"
    exit 1
fi
echo "%% automatically generated merged bibliography file" >lintel-latex-all.bib-new
echo >>lintel-latex-all.bib-new
for i in "$@"; do
    echo "% $DIR/$i" >>lintel-latex-all.bib-new
    cat $DIR/$i >>lintel-latex-all.bib-new
    echo >>lintel-latex-all.bib-new
    echo >>lintel-latex-all.bib-new
done

mv lintel-latex-all.bib-new lintel-latex-all.bib
