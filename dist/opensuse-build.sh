#!/bin/sh
SRPM=`redhat/get-srpm.sh $1`
[ -f $SRPM ]

. `dirname $0`/opensuse-setup-chroot.sh

$ZYPPER -R $ROOT install --auto-agree-with-licenses -t pattern -y devel_C_C++
/usr/local/sbin/rpm --root $ROOT -i $SRPM

if [ `ls $ROOT/usr/src/packages/SPECS/*.spec | wc -l` != 1 ]; then
    echo "Missing $ROOT/usr/src/packages/SPECS/*.spec"
    exit 1
fi

for i in `grep \^BuildRequires: $ROOT/usr/src/packages/SPECS/*.spec`; do
    if [ $i = BuildRequires: ]; then
        :
    else
        i=`echo $i | sed 's/,//'`
        REQUIRES="$REQUIRES $i"
    fi
done
# sleep 3600 || true
echo "Installing package requirements: $REQUIRES"
$ZYPPER -R $ROOT install -t package -y $REQUIRES

`dirname $0`/rpm-dump $ROOT

chroot $ROOT $LINUX32 /root/opensuse-chroot.sh --build

RESULT=/var/lib/mock/result/$1/rpms
mkdir -p $RESULT
cp $ROOT/usr/src/packages/RPMS/*/*.rpm $RESULT
echo "CP successful"
umount -f $ROOT/proc || true
rm -rf --one-file-system $ROOT || true
rmdir $ROOT || true
trap '' 0

exit 0
