#!/bin/sh
set -e -x
PB_ROOT=$1
MAKE_DIST=$2
. $MAKE_DIST/version.tmp
LINTEL=$PB_ROOT/Lintel/pbconf/$RELEASE_VERSION/Lintel
DATASERIES=$PB_ROOT/DataSeries/pbconf/$RELEASE_VERSION/DataSeries
[ -f $LINTEL/deb/changelog ]
[ -f $LINTEL/rpm/Lintel.spec ]
[ -f $DATASERIES/deb/changelog ]
[ -f $DATASERIES/rpm/DataSeries.spec ]
head -1 $LINTEL/deb/changelog
[ `head -1 $LINTEL/deb/changelog | grep $RELEASE_VERSION | wc -l` = 1 ]
[ `grep $RELEASE_VERSION $LINTEL/rpm/Lintel.spec | wc -l` = 1 ]
[ `head -1 $DATASERIES/deb/changelog | grep $RELEASE_VERSION | wc -l` = 1 ]
[ `grep $RELEASE_VERSION $DATASERIES/rpm/DataSeries.spec | wc -l` = 1 ]
