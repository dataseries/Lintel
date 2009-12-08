#!/bin/sh
#
# (c) Copyright 2009, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# shiny script because I can't figure out how to just do this in cmake.
set -e
OUTNAME="$1"
shift
DIR="$1"
shift
if [ ! -d "$DIR" ]; then
    echo "$DIR not a directory"
    exit 1
fi
echo "%% automatically generated merged bibliography file" >${OUTNAME}-new
echo >>${OUTNAME}-new
for i in "$@"; do
    echo "% $DIR/$i" >>${OUTNAME}-new
    cat $DIR/$i >>${OUTNAME}-new
    echo >>${OUTNAME}-new
    echo >>${OUTNAME}-new
done

mv ${OUTNAME}-new ${OUTNAME}
