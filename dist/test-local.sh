#!/bin/sh
set -e
BUILD=$1
NOW=$2
PATH=$BUILD/bin:$PATH
cd $BUILD
export PROJECTS=$BUILD/projects
export BUILD_OPT=$BUILD/build
rm -rf $PROJECTS $BUILD_OPT || true
perl $BUILD/deptool-bootstrap-local tarinit Lintel-$NOW.tar.bz2 DataSeries-$NOW.tar.bz2
cd $PROJECTS/DataSeries
perl $BUILD/deptool-bootstrap-local build -t
