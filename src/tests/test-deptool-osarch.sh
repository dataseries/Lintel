#!/bin/sh
set -e
set -x

# deptool should keep broken settings
BUILD_OS=broken
export BUILD_OS
eval `perl ../deptool-bootstrap getenv for-sh`
[ "$BUILD_OS" = broken ]

# unset should clear them out
eval `perl ../deptool-bootstrap getenv --unset for-sh`
eval `perl ../deptool-bootstrap getenv for-sh`
[ "$BUILD_OS" != broken ]
REF_OS=`echo $BUILD_OS | sed 's/-.*//'`
REF_VERSION=`echo $BUILD_OS | sed 's/.*-//'`
REF_ARCH=$UNAME_M
# easy, from env
./deptool-osarch $REF_OS $REF_VERSION $REF_ARCH

# still from env
BUILD_OS=test-ver
UNAME_M=magic
./deptool-osarch test ver magic

# From running deptool via env-path, UNAME_M still set
unset BUILD_OS
DEPTOOL=../deptool-bootstrap
export DEPTOOL
./deptool-osarch $REF_OS $REF_VERSION magic

# From running deptool via env-path, nothing set
unset UNAME_M
./deptool-osarch $REF_OS $REF_VERSION $REF_ARCH

export PERL5LIB=$1/src:$PERL5LIB
# From running deptool in path
unset DEPTOOL
PATH=`pwd`/..:$PATH
./deptool-osarch $REF_OS $REF_VERSION $REF_ARCH

