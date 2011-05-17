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
    x86_64) ZYPPER=zypper; BUILD=`pwd`/redhat/opensuse-chroot.sh ;;
    i586) ZYPPER="linux32 zypper"; BUILD="linux32 `pwd`/redhat/opensuse-chroot.sh" ;;
    *) echo "? $ARCH"; exit 1 ;;
esac

SRPM=`redhat/get-lintel-srpm.sh $1`

ROOT=/srv/chroot/opensuse-build/root
rm -rf $ROOT
HOME=$ROOT/root
mkdir -p $HOME
echo "%_topdir /usr/src/packages" >$HOME/.rpmmacros

zypper -R $ROOT addrepo http://mirrors2.kernel.org/opensuse/distribution/$VERSION/repo/oss/ "$VERSION-$ARCH-OSS"
$ZYPPER -R /srv/chroot/opensuse-build/root install --auto-agree-with-licenses -t pattern -y base devel_C_C++

/usr/local/sbin/rpm --root $ROOT -i $SRPM

REQUIRES="db45-utils" # for the db restore
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

schroot -c opensuse-build $BUILD

RESULT=/var/lib/mock/result/$1
mkdir -p $RESULT
rm $RESULT/*.rpm || true
cp $ROOT/usr/src/packages/RPMS/*/*.rpm $RESULT

exit 0
