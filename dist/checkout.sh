#!/bin/sh
BUILD=`pwd`
mtn -d $BUILD/tmp.db co -b ssd.hpl.hp.com/`echo $1 | sed 's/-.*//'` $1
cd $1
`dirname $0`/make-release-changelog.sh
