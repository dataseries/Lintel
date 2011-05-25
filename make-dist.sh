#!/bin/sh
PACKAGES="Lintel DataSeries"
# Add ts-rhel5 once it has cmake, Digest::SHA1
test_hosts() {
    awk '{print $1}' <<END_OF_HOST_LIST
keyvalue-debian-x86-2.u.hpl.hp.com # Debian Etch 32bit
keyvalue-d64-5.u.hpl.hp.com # Debian Lenny 64bit
hplxc.hpl.hp.com # RHEL4.5
ts-rhel5.hpl.hp.com # RHEL5.4
endpin.cs.hmc.edu # OpenSuSE 11.1
martenot.hpl.hp.com # Debian Lenny/Sid
END_OF_HOST_LIST
}
TEST_HOSTS=`test_hosts`

SCHROOT_ENVS="fedora12-64bit etch-32bit karmic-64bit lenny-32bit lenny-64bit"
[ "$MTN_PULL_FROM" = "" ] && MTN_PULL_FROM=usi.hpl.hp.com

REMOTE_TMP=/var/tmp

set -e

if [ "$1" = "--endpin-mtn" ]; then
    cd ~/projects/Lintel
    mtn sync endpin.cs.hmc.edu ssd.hpl.hp.com/Lintel\*
    cd ../DataSeries
    mtn sync endpin.cs.hmc.edu ssd.hpl.hp.com/DataSeries\*
    exit 0
fi

WWW=/var/www/external/tesla.hpl.hp.com/opensource
if [ "$1" = "--martenot-copy" -a "$2" != "" ]; then
    PROJ=$REMOTE_TMP/make-dist/projects
    cp $REMOTE_TMP/make-dist/deptool-bootstrap $WWW/deptool-bootstrap
    cp $PROJ/Lintel/NEWS $WWW/Lintel-NEWS.txt
    cp $PROJ/DataSeries/NEWS $WWW/DataSeries-NEWS.txt
    cp $PROJ/Lintel-$2.tar.bz2 $WWW/Lintel-$2.tar.bz2
    cp $PROJ/DataSeries-$2.tar.bz2 $WWW/DataSeries-$2.tar.bz2
    exit 0
fi

if [ "$1" = "--prepare" -a "$0" = "$REMOTE_TMP/make-dist.sh" ]; then
    [ ! -d $REMOTE_TMP/make-dist ] || mv $REMOTE_TMP/make-dist $REMOTE_TMP/make-dist.rm 
    [ ! -d $REMOTE_TMP/make-dist.rm ] || rm -rf $REMOTE_TMP/make-dist.rm 
    mkdir $REMOTE_TMP/make-dist 
    mv $0 $REMOTE_TMP/make-dist/make-dist.sh
    exit 0
fi

if [ "$1" = "--test-local" -a "$2" != "" -a "$3" != "" ]; then
    PATH=/tmp/make-dist/bin:$PATH
    cd /tmp/make-dist
    export PROJECTS=/tmp/make-dist/projects
    export BUILD_OPT=/tmp/make-dist/build
    rm -rf $PROJECTS $BUILD_OPT
    perl /tmp/make-dist/deptool-bootstrap tarinit Lintel-$2.tar.bz2 /tmp/make-dist/DataSeries-$2.tar.bz2
    cd $PROJECTS/DataSeries
    perl /tmp/make-dist/deptool-bootstrap build -t
    echo "MAKE-DIST: EVERYTHING OK!"
    echo $2 >ok-$3
    exit 0
fi

if [ "$1" = "--test-wget" -a "$2" != "" -a "$3" != "" ]; then
    PATH=$REMOTE_TMP/make-dist/bin:$PATH
    cd $REMOTE_TMP/make-dist
    export PROJECTS=$REMOTE_TMP/make-dist/projects
    export BUILD_OPT=$REMOTE_TMP/make-dist/build
    unset http_proxy
    unset no_proxy
    case $3 in
	*.hp.com) http_proxy=http://web-proxy.corp.hp.com:8088/;
	          export http_proxy ;;
    esac
    case $3 in
	# cmake
	hplxc.hpl.hp.com) PATH=$PATH:/hpl/home/anderse/.depot/rhel4-x86_64/bin ;;
	ts-rhel5.hpl.hp.com) PATH=$PATH:/home/anderse/.depot/rhel5-x86_64/bin ;;
    esac
    perl $REMOTE_TMP/make-dist/deptool-bootstrap --debug tarinit --no-cache http://tesla.hpl.hp.com/opensource/tmp/latest-release
    # Make sure we downloaded the right files
    [ -f $PROJECTS/Lintel-$2.tar.bz2 ]
    [ -f $PROJECTS/DataSeries-$2.tar.bz2 ]
    cd $PROJECTS/DataSeries
    perl $REMOTE_TMP/make-dist/deptool-bootstrap build -t
    echo "MAKE-DIST: EVERYTHING OK!"
    echo $2 >$REMOTE_TMP/make-dist/result-$3
    exit 0
fi

if [ "$1" != "" ]; then
    echo "Unexpected argument '$1' '$2' '$3'"
    echo "Usage: ./make-dist"
    exit 1
fi

LOG=/tmp/make-dist/log
NOW=`date +%Y-%m-%d`

echo "Current date: $NOW"
[ ! -d /tmp/make-dist ] || rm -rf /tmp/make-dist
mkdir /tmp/make-dist /tmp/make-dist/tar /tmp/make-dist/log
cp $0 /tmp/make-dist
cd /tmp/make-dist/tar

[ -f tmp.db ] || mtn -d tmp.db db init
touch pull.log
for i in $PACKAGES; do
    echo "Pulling $i from usi.hpl.hp.com; logging to /tmp/make-dist/pull.log..."

    cp $HOME/.monotone/ssd.db tmp.db
#    mtn -d tmp.db pull $MTN_PULL_FROM ssd.hpl.hp.com/$i >>pull.log 2>&1
    HEAD="`mtn -d tmp.db automate heads ssd.hpl.hp.com/$i`"
    REF_HEAD="`cd ~/projects/$i; mtn automate heads`"
    if [ "$HEAD" != "$REF_HEAD" ]; then 
	echo "Weird, heads differ between synced ($HEAD) and ~/projects/$i ($REF_HEAD)"
	exit 1
    fi
done

ssh martenot.hpl.hp.com $REMOTE_TMP/make-dist/make-dist.sh --martenot-copy $NOW
scp /tmp/make-dist/latest-release martenot:$WWW/latest-release

echo "Update the html, and resync to tesla, and we're ready to go."

exit 0

