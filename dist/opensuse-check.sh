#!/bin/sh
. `dirname $0`/opensuse-setup-chroot.sh

$ZYPPER -R $ROOT install -t package -y zypper
`dirname $0`/rpm-dump $ROOT

cp `dirname $0`/check-packages.sh $ROOT/check-packages.sh
chroot $ROOT $LINUX32 /root/opensuse-chroot.sh --check

