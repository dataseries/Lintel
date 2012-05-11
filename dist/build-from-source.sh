#!/bin/sh
set -e
set -x

if [ $# != 6 ]; then
    echo "Usage: $0 <release-version> <hostname-for-build> <release-url-prefix> <lintel-sha1> <dataseries-sha1> <examples-sha1>"
    exit 1
fi

VERSION=$1
MY_HOST=$2
URL_PREFIX=$3
LINTEL_SHA=$4
DATASERIES_SHA=$5
EXAMPLES_SHA=$6

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

fetchUrl() {
    if [ `which curl | wc -l` -ge 1 ]; then
        curl $1 >`basename $1`
    elif [ `which wget | wc -l` -ge 1 ]; then
        [ ! -f `basename $1` || rm -f `basename $1` ]
        wget --no-cache $1
    else
        echo "No wget or curl?"
        exit 1
    fi
}

fetchUrl $URL_PREFIX/deptool-bootstrap

# Deptool
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

# TODO: merge this with test-example-build.sh in some way.
(
    set -e
    cd $HOME
    RELEASE_VERSION=$VERSION
    OLD_VERSION=2011-06-13

    downloadUntar() {
        [ ! -f DataSeriesExamples-$1.tar.gz ] || rm DataSeriesExamples-$1.tar.gz
        fetchUrl $URL_PREFIX/DataSeriesExamples-$1.tar.gz
        [ ! -d DataSeriesExamples-$1 ] || rm -rf DataSeriesExamples-$1
        tar xvvfz DataSeriesExamples-$1.tar.gz
    }

    downloadUntar $OLD_VERSION
    downloadUntar $RELEASE_VERSION
    [ `$SHA1SUM < DataSeriesExamples-$RELEASE_VERSION.tar.gz | awk '{print $1}'` = $EXAMPLES_SHA ]
    touch /tmp/downloaded.$VERSION
)

[ -f /tmp/downloaded.$VERSION ] && rm /tmp/downloaded.$VERSION

# Build and test
cd $PROJECTS/DataSeries
perl $REMOTE_TMP/deptool-bootstrap build -t

(
    set -e
    cd $HOME
    LD_LIBRARY_PATH=$BUILD_OPT/lib:$LD_LIBRARY_PATH

    RELEASE_VERSION=$VERSION
    OLD_VERSION=2011-06-13
    START_DIR=`pwd`

    case $BUILD_OS in
        *bsd*) SIMPLE_FLAGS="BOOST_LIB=-L/usr/local/lib" ;;
        *macos*) SIMPLE_FLAGS="LIBXML2_INCLUDE=-I/usr/include/libxml2" ;;
    esac

    buildOld() {
        cd DataSeriesExamples-$OLD_VERSION/triangle-find
        case $BUILD_OS in 
            *openbsd*) : ;; # no threads on this platform, no triangle find
                            # no pattern rules in make, not going to fix.
            *)
                cmake -D CMAKE_INSTALL_PREFIX=$BUILD_OPT $DEPTOOL_CMAKE_FLAGS .
                make && ctest
                cd ../simple
                make PREFIX=$BUILD_OPT $SIMPLE_FLAGS
                ;;
        esac
        cd $START_DIR
    }
    
    buildNew() {
        cd DataSeriesExamples-$1/triangle-find
        
        case $BUILD_OS in
            *openbsd*) : ;; # no threads on this platform, no triangle find.
                            # no pattern rules in make, not going to fix.
            *)
                cmake -D CMAKE_INSTALL_PREFIX=$BUILD_OPT .
                make  && ctest
                cd ../simple
                make PREFIX=$BUILD_OPT $SIMPLE_FLAGS
                make test PREFIX=$BUILD_OPT
                ;;
        esac

        cd $START_DIR
    }

    buildOld
    buildNew $RELEASE_VERSION
    rm -rf $HOME/DataSeriesExamples-$OLD_VERSION.tar.gz $HOME/DataSeriesExamples-$OLD_VERSION
    rm -rf $HOME/DataSeriesExamples-$RELEASE_VERSION.tar.gz $HOME/DataSeriesExamples-$RELEASE_VERSION
    touch /tmp/built.$VERSION
)

[ -f /tmp/built.$VERSION ] && rm /tmp/built.$VERSION

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
