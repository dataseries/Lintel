#!/bin/sh
PB_ROOT=$1
TRIPLE=$2
RELEASE_VERSION=$3

sudo cp dist/test-example-bootstrap.sh dist/test-example-build.sh $PB_ROOT/chroot/`echo $TRIPLE | sed 's,-,/,g'`/

case $TRIPLE in
    *i386) sudo linux32 chroot $PB_ROOT/chroot/`echo $TRIPLE | sed 's,-,/,g'` /bin/sh /test-example-bootstrap.sh $TRIPLE $RELEASE_VERSION ;;
    *x86_64) sudo chroot $PB_ROOT/chroot/`echo $TRIPLE | sed 's,-,/,g'` /bin/sh /test-example-bootstrap.sh $TRIPLE $RELEASE_VERSION ;;
    *) exit 1
esac

