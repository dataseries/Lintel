#!/bin/sh
set -e
. redhat/get-version.sh
SRC=$rpm_topdir/SRPMS/$1-$VERSION-$RELEASE.src.rpm 
for i in fedora centos opensuse scilinux; do
    perl redhat/patch-spec.pl $i $VERSION $RELEASE >/tmp/$1.spec
    rpmbuild -bs --nodeps /tmp/$1.spec
    DEST=$rpm_topdir/SRPMS/$1-$i-$VERSION-$RELEASE.src.rpm 
    mv $SRC $DEST
    rm /tmp/$1.spec
    echo "Created $i -- $DEST"
done
	
