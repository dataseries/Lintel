#!/bin/sh
set -e
. redhat/get-version.sh
SRC=$rpm_topdir/SRPMS/Lintel-$VERSION-$RELEASE.src.rpm 
for i in fedora centos; do
    perl redhat/patch-spec.pl $i $VERSION $RELEASE
    rpmbuild -bs --nodeps redhat/Lintel.spec
    mv $SRC $rpm_topdir/SRPMS/Lintel-$i-$VERSION-$RELEASE.src.rpm 
done
	
