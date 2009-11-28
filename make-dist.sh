#!/bin/sh
PACKAGES="Lintel DataSeries"
TEST_HOSTS="keyvalue-debian-x86-2.u.hpl.hp.com pds-big-1.u.hpl.hp.com hplxc.hpl.hp.com ts-rhel5.hpl.hp.com endpin.cs.hmc.edu"
[ "$MTN_PULL_FROM" = "" ] && MTN_PULL_FROM=usi.hpl.hp.com

set -e

if [ "$1" = "--endpin-mtn" ]; then
    cd ~/projects/Lintel
    mtn sync endpin.cs.hmc.edu ssd.hpl.hp.com/Lintel\*
    cd ../DataSeries
    mtn sync endpin.cs.hmc.edu ssd.hpl.hp.com/DataSeries\*
    exit 0
fi

if [ "$1" = "--prepare" -a "$0" = "/tmp/make-dist.sh" ]; then
    [ ! -d /tmp/make-dist ] || mv /tmp/make-dist /tmp/make-dist.rm 
    [ ! -d /tmp/make-dist.rm ] || rm -rf /tmp/make-dist.rm 
    mkdir /tmp/make-dist 
    mv $0 /tmp/make-dist/make-dist.sh
    exit 0
fi

if [ "$1" = "--test-local" -a "$2" != "" -a "$3" != "" ]; then
    PATH=/tmp/make-dist/bin:$PATH
    cd /tmp/make-dist
    export PROJECTS=/tmp/make-dist/projects
    export BUILD_OPT=/tmp/make-dist/build
    perl /tmp/make-dist/deptool-bootstrap tarinit Lintel-$2.tar.bz2 /tmp/make-dist/DataSeries-$2.tar.bz2
    cd $PROJECTS/DataSeries
    perl /tmp/make-dist/deptool-bootstrap build -t
    echo "MAKE-DIST: EVERYTHING OK!"
    echo $2 >ok-$3
    exit 0
fi

if [ "$1" = "--test-wget" -a "$2" != "" -a "$3" != "" ]; then
    PATH=/tmp/make-dist/bin:$PATH
    cd /tmp/make-dist
    export PROJECTS=/tmp/make-dist/projects
    export BUILD_OPT=/tmp/make-dist/build
    unset http_proxy
    unset no_proxy
    case $3 in
	*.hp.com) http_proxy=http://web-proxy.corp.hp.com:8088/;
	   export http_proxy ;;
    esac
    perl /tmp/make-dist/deptool-bootstrap --debug tarinit http://tesla.hpl.hp.com/opensource/tmp/latest-release
    cd $PROJECTS/DataSeries
    perl /tmp/make-dist/deptool-bootstrap build -t
    echo "MAKE-DIST: EVERYTHING OK!"
    echo $2 >/tmp/make-dist/result-$3
    exit 0
fi

if [ "$1" != "" ]; then
    echo "Unexpected argument '$1' '$2' '$3'"
    echo "Usage: ./make-dist"
    exit 1
fi

LOG=/tmp/make-dist/log
NOW=`date +%Y-%m-%d`

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

for i in $PACKAGES; do
    echo "Packaging $i: "
    [ ! -d $i-$NOW ] || rm -rf $i-$NOW
    mtn -d tmp.db co -b ssd.hpl.hp.com/$i $i-$NOW >../log/co.$i 2>&1
    cd $i-$NOW
    mtn log >Changelog.mtn
    echo "Monotone-Revision: `mtn automate get_base_revision_id`" >Release.info
    echo "Creation-Date: $NOW" >>Release.info
    echo "   current revision is `grep Monotone-Revision Release.info | awk '{print $2}'`"
    cd ..
    echo "   tarfile; building $i-$NOW.tar.bz2; logging to /tmp/make-dist/tar.$i.log"
    tar cvvfj $i-$NOW.tar.bz2 --exclude=_MTN $i-$NOW >tar.$i.log 2>&1
    echo "   done: `ls -l $i-$NOW.tar.bz2`"
done

# Local build
echo "setup files..."
cp $BUILD_OPT/Lintel/src/deptool-bootstrap /tmp/make-dist
cp Lintel-$NOW.tar.bz2 /tmp/make-dist
cp DataSeries-$NOW.tar.bz2 /tmp/make-dist

echo "Copy to tesla (bg)..."
echo "Lintel-$NOW.tar.bz2" >/tmp/make-dist/latest-release
echo "DataSeries-$NOW.tar.bz2" >>/tmp/make-dist/latest-release

rsync -av --progress /tmp/make-dist/Lintel-$NOW.tar.bz2 /tmp/make-dist/DataSeries-$NOW.tar.bz2 /tmp/make-dist/latest-release tesla.hpl.hp.com:opensource/tmp >$LOG/rsync 2>&1 &

echo "Local build..."
/tmp/make-dist/make-dist.sh --test-local $NOW localhost >$LOG/build.localhost 2>&1

cp /tmp/make-dist/build/Lintel/src/deptool-bootstrap /tmp/make-dist

echo "Waiting for copy to finish..."
wait

echo "starting test-host builds..."

for host in $TEST_HOSTS; do
    echo "Starting on $host; log to $LOG/$host.log"
    echo -n "  copy..."
    scp /tmp/make-dist/make-dist.sh $host:/tmp/make-dist.sh >$LOG/$host.log 2>&1
    echo -n "  prepare..."
    ssh $host /tmp/make-dist.sh --prepare >$LOG/$host.log 2>&1
    echo -n "  copy..."
    scp /tmp/make-dist/deptool-bootstrap $host:/tmp/make-dist/deptool-bootstrap >>$LOG/$host.log 2>&1

    echo "start build."

    ssh $host /tmp/make-dist/make-dist.sh --test-wget $NOW $host >>$LOG/$host.log 2>&1 &
done

# echo "schroot builds..."
# for i in etch-32bit lenny-32bit; do
#     echo "$i..."
#     schroot -c $i -- /tmp/make-dist/make-dist.sh --test-local $NOW $i >$LOG/$i 2>&1
# done

wait

echo "verifying test-host builds succeeded."
for host in $TEST_HOSTS; do
    echo -n "$host: fetch..."
    echo "No-File-Copied" >$LOG/result-$host
    scp $host:/tmp/make-dist/result-$host $LOG/result-$host >>$LOG/$host.log 2>&1 || true
    GOT="`head -1 $LOG/result-$host`"
    if [ "$GOT" = "$NOW" ]; then
	echo "ok"
    else
	echo "failed, ok-$host doesn't contain $NOW, but '$GOT'"
	exit 1
    fi
done

exit 0

