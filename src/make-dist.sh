#!/bin/sh
PACKAGES="Lintel DataSeries"
CLEAN=false

# TODO: test on additional hosts after building tarballs...

set -e
if $CLEAN; then
    [ ! -d /tmp/make-dist ] || rm -rf /tmp/make-dist
fi
[ -d /tmp/make-dist ] || mkdir /tmp/make-dist
cd /tmp/make-dist

[ -f tmp.db ] || mtn -d tmp.db db init
touch pull.log
for i in $PACKAGES; do
    echo "Pulling $i from usi.hpl.hp.com; logging to pull.log..."

    mtn -d tmp.db pull usi.hpl.hp.com ssd.hpl.hp.com/$i >>pull.log 2>&1
done

PROCESSORS=`cat /proc/cpuinfo | grep processor | tail -1 | awk '{print $3}'`
PROCESSORS=`expr $PROCESSORS + 1`
[ -d build ] || mkdir build
PATH=/tmp/make-dist/build/bin:$PATH
NOW=`date +%Y-%m-%d`
for i in $PACKAGES; do
    echo "Packaging $i: "
    [ ! -d $i-$NOW ] || rm -rf $i-$NOW
    mtn -d tmp.db co -b ssd.hpl.hp.com/$i $i-$NOW
    cd $i-$NOW
    mtn automate get_base_revision_id >Revision
    echo "   autoreconf; logging to autoreconf.$i.log"
    [ $i = Lintel ] || /tmp/make-dist/build/bin/lintel-acinclude >../autoreconf.$i.log
    autoreconf --install >>autoreconf.$i.log 2>&1
    cd ..
    [ ! -d build/$i ] || rm -rf build/$i
    mkdir build/$i
    cd build/$i
    echo "   configure; logging to build/$i/configure.log"
    ../../$i-$NOW/configure --prefix=/tmp/make-dist/build --enable-optmode=optimize >configure.log 2>&1
    echo "   compiling (-j $PROCESSORS); logging to build/$i/compile.log"
    make -j $PROCESSORS >compile.log 2>&1
    echo "   checking; logging to build/$i/check.log"
    make -j $PROCESSORS check >check.log 2>&1
    echo "   installing; logging to build/$i/install.log"
    make install >install.log 2>&1
    cd ../..
    echo "   tarfile; building $i-$NOW.tar.bz2; logging to tar.$i.log"
    tar cvvfj $i-$NOW.tar.bz2 $i-$NOW >tar.$i.log 2>&1
    echo "   done: `ls -l $i-$NOW.tar.bz2`"
done

