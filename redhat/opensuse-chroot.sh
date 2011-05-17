#!/bin/sh
cd /var/lib/rpm
rm __db.*
for i in *.dump; do
    j=`basename $i .dump`
    mv $j $j.orig
    echo "restore $j from $i"
    db45_load $j < $i
done

uname -a
rpmbuild -ba /usr/src/packages/SPECS/Lintel.spec
