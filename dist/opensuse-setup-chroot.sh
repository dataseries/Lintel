#!/bin/sh
set -e
set -x
[ -f /etc/sysconfig/proxy ] # for HTTP_PROXY=http://localhost:3128

if [ -z "$1" ]; then
    echo "Usage: $0 os-version-arch"
    exit 1
fi

array=(${1//-/ })
OS=${array[0]}
VERSION=${array[1]}
ARCH=${array[2]}

[ $OS = opensuse ]

case $ARCH in
    x86_64) ZYPPER=zypper; LINUX32= ;;
    i586) ZYPPER="linux32 zypper"; LINUX32=linux32 ;;
    *) echo "? $ARCH"; exit 1 ;;
esac

ROOT=/srv/chroot/opensuse-build/`date +%Y-%m-%d--$$`/root
trap 'umount -f $ROOT/proc || true; sleep 1; rm -rf --one-file-system $ROOT || true; rmdir $ROOT || true' 0
umount -f $ROOT/proc || true
rm -rf --one-file-system $ROOT
HOME=$ROOT/root
ZYPP_LOCKFILE_ROOT=$HOME
export ZYPP_LOCKFILE_ROOT
mkdir -p $HOME
echo "%_topdir /usr/src/packages" >$HOME/.rpmmacros

$ZYPPER -R $ROOT addrepo http://mirrors1.kernel.org/opensuse/distribution/$VERSION/repo/oss/ "$VERSION-$ARCH-OSS"
if [ ! -d /var/www/localpkgs/$1 ]; then
    echo "Missing /var/www/localpkgs/$1"
fi
$ZYPPER -R $ROOT addrepo http://localhost/localpkgs/$1 "localpkgs"
$ZYPPER -R $ROOT refresh
$ZYPPER -R $ROOT install --auto-agree-with-licenses -t pattern -y base

case $VERSION in
    11.[23]) REQUIRES=db-utils ;; 
    *) REQUIRES=db45-utils ;; # for the db restore
esac

$ZYPPER -R $ROOT install -t package -y $REQUIRES
cp /dev/MAKEDEV $ROOT/dev/MAKEDEV
cp `dirname $0`/opensuse-chroot.sh $ROOT/root/opensuse-chroot.sh

