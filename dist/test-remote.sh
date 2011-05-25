#!/bin/sh
REMOTE_TMP=/var/tmp
set -e
NOW=$1
HOST=$2

[ ! -d $REMOTE_TMP/test-remote ] || mv $REMOTE_TMP/test-remote $REMOTE_TMP/test-remote.rm 
[ ! -d $REMOTE_TMP/test-remote.rm ] || rm -rf $REMOTE_TMP/test-remote.rm 
mkdir $REMOTE_TMP/test-remote 
mv $0 $REMOTE_TMP/test-remote/test-remote.sh

PATH=$REMOTE_TMP/test-remote/build/bin:$PATH
cd $REMOTE_TMP/test-remote
export PROJECTS=$REMOTE_TMP/test-remote/projects
export BUILD_OPT=$REMOTE_TMP/test-remote/build
unset http_proxy
unset no_proxy
case $HOST in
    *.hp.com) http_proxy=http://web-proxy.corp.hp.com:8088/;
              export http_proxy ;;
esac
case $HOST in
    # cmake
    hplxc.hpl.hp.com) PATH=$PATH:/hpl/home/anderse/.depot/rhel4-x86_64/bin ;;
    ts-rhel5.hpl.hp.com) PATH=$PATH:/home/anderse/.depot/rhel5-x86_64/bin ;;
esac
wget http://tesla.hpl.hp.com/opensource/tmp/deptool-bootstrap
perl $REMOTE_TMP/test-remote/deptool-bootstrap --debug tarinit --no-cache http://tesla.hpl.hp.com/opensource/tmp/latest-release
# Make sure we downloaded the right files
[ -f $PROJECTS/Lintel-$NOW.tar.bz2 ]
[ -f $PROJECTS/DataSeries-$NOW.tar.bz2 ]
cd $PROJECTS/DataSeries
perl $REMOTE_TMP/test-remote/deptool-bootstrap build -t
echo "TEST-REMOTE: EVERYTHING OK!"
echo $NOW >$REMOTE_TMP/test-remote/result-$HOST
