#!/bin/sh
set -e
. redhat/get-version.sh
SRC=$rpm_topdir/SRPMS/$1-$VERSION-$RELEASE.src.rpm 
for i in fedora centos opensuse scilinux; do
    perl redhat/patch-spec.pl $i $VERSION $RELEASE
    rpmbuild -bs --nodeps redhat/$1.spec
    DEST=$rpm_topdir/SRPMS/$1-$i-$VERSION-$RELEASE.src.rpm 
    mv $SRC $DEST
    echo "Created $i -- $DEST"
done
	
