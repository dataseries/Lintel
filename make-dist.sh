#!/bin/sh
PACKAGES="Lintel DataSeries"
[ "$CLEAN" = "" ] && CLEAN=true
TEST_HOSTS="batch-master-1.u.hpl.hp.com anderse-tengig-1.u.hpl.hp.com"
[ "$MTN_PULL_FROM" = "" ] && MTN_PULL_FROM=usi.hpl.hp.com

set -e
PROCESSORS=`cat /proc/cpuinfo | grep processor | tail -1 | awk '{print $3}'`
PROCESSORS=`expr $PROCESSORS + 1`
if [ "$1" = "--test" -a "$2" != "" ]; then
    PATH=/tmp/make-dist/bin:$PATH
    cd /tmp/make-dist
    mkdir build
    for i in $PACKAGES; do
	echo "MAKE-DIST: unpack $i"
	tar xvvfj $i-$2.tar.bz2
	mkdir build/$i
	cd build/$i
	echo "MAKE-DIST: configure $i"
	../../$i-$2/configure --prefix=/tmp/make-dist 
	echo "MAKE-DIST: compile $i"
	make -j $PROCESSORS
	echo "MAKE-DIST: check $i"
	make -j $PROCESSORS check
	echo "MAKE-DIST: install $i"
	make install
        cd ../..
    done
    echo "MAKE-DIST: EVERYTHING OK!"
    echo $2 >ok-$3
    exit 0
fi

if $CLEAN; then
    [ ! -d /tmp/make-dist ] || rm -rf /tmp/make-dist
fi
[ -d /tmp/make-dist ] || mkdir /tmp/make-dist
cp $0 /tmp/make-dist
cd /tmp/make-dist

[ -f tmp.db ] || mtn -d tmp.db db init
touch pull.log
for i in $PACKAGES; do
    echo "Pulling $i from usi.hpl.hp.com; logging to pull.log..."

    mtn -d tmp.db pull $MTN_PULL_FROM ssd.hpl.hp.com/$i >>pull.log 2>&1
done

[ -d build ] || mkdir build
PATH=/tmp/make-dist/build/bin:$PATH
NOW=`date +%Y-%m-%d`
for i in $PACKAGES; do
    echo "Packaging $i: "
    [ ! -d $i-$NOW ] || rm -rf $i-$NOW
    mtn -d tmp.db co -b ssd.hpl.hp.com/$i $i-$NOW
    cd $i-$NOW
    mtn log >Changelog.mtn
    echo "Monotone-Revision: `mtn automate get_base_revision_id`" >Release.info
    echo "Creation-Date: $NOW" >>Release.info
    echo "   current revision is `grep Monotone-Revision Release.info | awk '{print $2}'`"
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
    tar cvvfj $i-$NOW.tar.bz2 --exclude=_MTN $i-$NOW >tar.$i.log 2>&1
    echo "   done: `ls -l $i-$NOW.tar.bz2`"
done

for host in $TEST_HOSTS; do
    echo "Testing on $host; logging everything to test-$host.log:"
    ssh $host rm -rf /tmp/make-dist >$host.log 2>&1
    ssh $host mkdir /tmp/make-dist >>$host.log 2>&1
    scp $0 $host:/tmp/make-dist
    
    for package in $PACKAGES; do
	echo "   copy $package..."
	scp $package-$NOW.tar.bz2 $host:/tmp/make-dist/$package-$NOW.tar.bz2 >>$host.log 2>&1
    done
    echo "   test build/check/install..."
    ssh $host /tmp/make-dist/make-dist.sh --test $NOW $host >>$host.log 2>&1
    echo -n "   verify completed correctly: "
    scp $host:/tmp/make-dist/ok-$host ok-$host >>$host.log 2>&1
    if [ "`cat ok-$host`" = "$NOW" ]; then
	echo "ok"
    else
	echo "failed, ok-$host doesn't contain $NOW"
	exit 1
    fi
done

