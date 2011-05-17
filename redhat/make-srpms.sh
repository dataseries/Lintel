#!/bin/sh
set -e
. redhat/get-version.sh
SRC=$rpm_topdir/SRPMS/Lintel-$VERSION-$RELEASE.src.rpm 
for i in fedora centos opensuse; do
    perl redhat/patch-spec.pl $i $VERSION $RELEASE
    rpmbuild -bs --nodeps redhat/Lintel.spec
    DEST=$rpm_topdir/SRPMS/Lintel-$i-$VERSION-$RELEASE.src.rpm 
    mv $SRC $DEST
    echo "Created $i -- $DEST"
done
	
