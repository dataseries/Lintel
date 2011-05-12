#!/bin/sh
. redhat/get-version.sh

echo "Using rpm_topdir = $rpm_topdir"

cp ../Lintel-$VERSION.tar.gz $rpm_topdir/SOURCES/Lintel-$VERSION.tar.gz
