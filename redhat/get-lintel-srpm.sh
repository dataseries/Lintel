#!/bin/sh
. redhat/get-version.sh
OS=`echo $1 | sed 's/-.*//'`
echo "$rpm_topdir/SRPMS/Lintel-$OS-$VERSION-1.src.rpm"
