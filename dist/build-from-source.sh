#!/bin/sh
set -e
set -x

if [ $# != 5 ]; then
    echo "Usage: $0 <release-version> <hostname-for-build> <release-url-prefix> <lintel-sha1> <dataseries-sha1>"
    exit 1
fi

VERSION=$1
MY_HOST=$2
URL_PREFIX=$3
LINTEL_SHA=$4
DATASERIES_SHA=$5

REMOTE_TMP=/var/tmp/dataseries-test

# Setup environment
unset http_proxy
unset no_proxy
case $MY_HOST in
    *.hp.com) http_proxy=http://web-proxy.corp.hp.com:8088/;
              export http_proxy ;;
    # default KVM open
    openbsd-*-*) REMOTE_TMP=/home/anderse/dataseries-test ;;
esac
case $LATEST_RELEASE in
    *192.168.122.1*) http_proxy=http://192.168.122.1:3128/; no_proxy=192.168.122.1; 
        export http_proxy; export no_proxy ;;
esac

# Prepare working directory
[ ! -d $REMOTE_TMP ] || rm -rf $REMOTE_TMP
mkdir $REMOTE_TMP
cd $REMOTE_TMP

# Deptool
if [ `which curl | wc -l` -ge 1 ]; then
    curl $URL_PREFIX/deptool-bootstrap >deptool-bootstrap
elif [ `which wget | wc -l` -ge 1 ]; then
    wget --no-cache $URL_PREFIX/deptool-bootstrap
else
    echo "No wget or curl?"
    exit 1
fi
eval `perl deptool-bootstrap getenv --unset for-sh`
eval `env PROJECTS=$REMOTE_TMP/projects BUILD_ROOT=$REMOTE_TMP perl deptool-bootstrap getenv for-sh`
PATH=$BUILD_OPT/bin:$HOME/.depot/$BUILD_OS-$UNAME_M/bin:$PATH

# Get sources
perl deptool-bootstrap init --tar --no-cache $URL_PREFIX/latest-release

## verify we got the right version
[ -f $PROJECTS/Lintel-$VERSION.tar.bz2 ]
[ -d $PROJECTS/Lintel ]
[ -f $PROJECTS/DataSeries-$VERSION.tar.bz2 ]
[ -d $PROJECTS/DataSeries ]

[ `which sha1 2>/dev/null | wc -l` = 0 ] || SHA1SUM=sha1
[ `which shasum 2>/dev/null | wc -l` = 0 ] || SHA1SUM=shasum
[ `which sha1sum 2>/dev/null | wc -l` = 0 ] || SHA1SUM=sha1sum

[ "$SHA1SUM" != "" ]

[ `$SHA1SUM < $PROJECTS/Lintel-$VERSION.tar.bz2 | awk '{print $1}'` = $LINTEL_SHA ]
[ `$SHA1SUM < $PROJECTS/DataSeries-$VERSION.tar.bz2 | awk '{print $1}'` = $DATASERIES_SHA ]

# Build and test
cd $PROJECTS/DataSeries
perl $REMOTE_TMP/deptool-bootstrap build -t

# Note success
case $MY_HOST in
    openbsd-*-*) rm -rf $REMOTE_TMP ; 
        REMOTE_TMP=/var/tmp/dataseries-test ;
        [ ! -d $REMOTE_TMP ] || rm -rf $REMOTE_TMP ;
        mkdir $REMOTE_TMP ;
        ;;
esac
        
mv $0 $REMOTE_TMP/`basename $0` # for single thing to rm
echo "TEST-REMOTE: EVERYTHING OK!"
echo $VERSION >$REMOTE_TMP/result-$MY_HOST
