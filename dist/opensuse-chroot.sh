#!/bin/sh
set -e
if [ ! -e /proc/$$ ]; then
    mount -t proc proc /proc
fi
trap 'umount /proc || true' 0
cd /dev
./MAKEDEV null
./MAKEDEV zero

cd /var/lib/rpm
rm __db.* || true

if [ -x /usr/bin/db45_load ]; then
    LOAD=/usr/bin/db45_load
else
    LOAD=/usr/bin/db_load
fi

for i in *.dump; do
    j=`basename $i .dump`
    mv $j $j.orig
    echo "restore $j from $i"
    $LOAD $j < $i
done

uname -a

if [ "$1" = "--check" ]; then
    /check-packages.sh --zypper
elif [ "$1" = "--build" ]; then
    chown root /usr/src/packages/*/*
    chgrp root /usr/src/packages/*/*
    rpmbuild -ba /usr/src/packages/SPECS/*.spec
    echo "Successful build!"
else
    echo "? '$1'"
    exit 1
fi

