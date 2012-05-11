#!/bin/sh
set -x
set -e
if [ $# != 2 ]; then
    echo "Usage: $0 <os-ver-arch> <release-version>"
    exit 1
fi

TRIPLE=$1
RELEASE_VERSION=$2

set -- `echo $TRIPLE | tr -- '-' ' '`
OS=$1
VER=$2
ARCH=$3

[ -d /proc/$$ ] || mount -t proc proc /proc
rm /home/test-user || true
deluser test-user || true
[ `grep test-user /etc/passwd | wc -l` = 1 ] || yes y | useradd -m test-user

BASEURL=http://localhost/pb-pkgs/production
http_proxy=http://localhost:3128/
export http_proxy
debInstall() {
    wget -O /etc/apt/sources.list.d/Dataseries.sources.list $BASEURL/$1/DataSeries.sources.list
    wget -O /tmp/key $BASEURL/$1/DataSeries.pubkey
    apt-key add /tmp/key
    apt-get update
    apt-get install -y libdataseries-dev dataseries-utils build-essential cmake pkg-config libboost-regex-dev
}

yumInstall() {
    wget -O /etc/yum.repos.d/DataSeries.repo $BASEURL/$1/DataSeries.repo
    wget -O /tmp/key $BASEURL/$1/DataSeries.pubkey
    case $OS-$VER in
        centos-6.*) rpm --import /tmp/key || true ;; # centos 6 can mysteriously fail, but actually install the key
        *) rpm --import /tmp/key ;;
    esac
    case $OS-$VER in
        centos-5.[23456]) 
            wget -O /tmp/cmake.rpm http://dl.fedoraproject.org/pub/epel/5/$ARCH/cmake-2.6.4-5.el5.2.$ARCH.rpm
            [ `rpm -q -a | grep cmake | wc -l` = 1 ] || rpm -i /tmp/cmake.rpm ;;
        *) yum -y install cmake ;;
    esac
    yum -y install DataSeries-devel DataSeries-utils gcc-c++ pkgconfig diffutils
}

zypperInstall() {
    wget -O /tmp/DataSeries.repo $BASEURL/$1/DataSeries.repo
    [ `zypper repos | grep DataSeries | wc -l` = 1 ] || zypper addrepo /tmp/DataSeries.repo
    wget -O /tmp/key $BASEURL/$1/DataSeries.pubkey
    rpm --import /tmp/key || true
    zypper --gpg-auto-import-keys -n install DataSeries-devel DataSeries-utils gcc-c++ cmake pkg-config
}

case $OS in
    debian|ubuntu) 
        debInstall $OS/$VER
        ;;
    centos|fedora|scilinux)
        yumInstall $OS/$VER/$ARCH
        ;;
    opensuse)
        zypperInstall $OS/$VER/$ARCH
        ;;
    *) echo "Unknown OS '$OS'"; exit 1 ;;
esac

cp /test-example-build.sh /home/test-user/test-example-build.sh
su -l -c "/bin/sh /home/test-user/test-example-build.sh $RELEASE_VERSION $TRIPLE" test-user 
umount /proc
echo "SUCCESS BUILDING EXAMPLES"
