#!/bin/sh
set -e
set -x
[ -f /etc/sysconfig/proxy ] # for HTTP_PROXY=http://localhost:3128
[ ! -z "$http_proxy" ]

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

SRPM=`redhat/get-lintel-srpm.sh $1`
[ -f $SRPM ]

ROOT=/srv/chroot/opensuse-build/root
umount -f $ROOT/proc || true
rm -rf --one-file-system $ROOT
HOME=$ROOT/root
mkdir -p $HOME
echo "%_topdir /usr/src/packages" >$HOME/.rpmmacros

zypper -R $ROOT addrepo http://mirrors2.kernel.org/opensuse/distribution/$VERSION/repo/oss/ "$VERSION-$ARCH-OSS"
$ZYPPER -R /srv/chroot/opensuse-build/root install --auto-agree-with-licenses -t pattern -y base devel_C_C++

/usr/local/sbin/rpm --root $ROOT -i $SRPM

case $VERSION in
    11.[23]) REQUIRES=db-utils ;; 
    *) REQUIRES=db45-utils ;; # for the db restore
esac

for i in `grep \^BuildRequires: $ROOT/usr/src/packages/SPECS/Lintel.spec`; do
    if [ $i = BuildRequires: ]; then
        :
    else
        i=`echo $i | sed 's/,//'`
        REQUIRES="$REQUIRES $i"
    fi
done
echo "Installing package requirements: $REQUIRES"
$ZYPPER -R $ROOT install -t package -y $REQUIRES

for i in `file $ROOT/var/lib/rpm/* | grep 'Berkeley DB' | grep -v .orig | awk '{print $1}' | sed 's/:$//'`; do
    echo "Dumping $i..."
    db4.6_dump $i >$i.dump
done

cp /dev/MAKEDEV $ROOT/dev/MAKEDEV
cp `pwd`/redhat/opensuse-chroot.sh $ROOT/root/opensuse-chroot.sh
cd /
schroot -c opensuse-build $LINUX32 /root/opensuse-chroot.sh

RESULT=/var/lib/mock/result/$1
mkdir -p $RESULT
rm $RESULT/*.rpm || true
cp $ROOT/usr/src/packages/RPMS/*/*.rpm $RESULT

exit 0
