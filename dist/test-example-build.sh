#!/bin/sh
set -e
RELEASE_VERSION=$1
BUILD_OS=$2
OLD_VERSION=2011-06-13
START_DIR=`pwd`

downloadUntar() {
    [ ! -f DataSeriesExamples-$1.tar.gz ] || rm DataSeriesExamples-$1.tar.gz
    wget --no-cache http://localhost/pb-sources/DataSeriesExamples-$1.tar.gz
    [ ! -d DataSeriesExamples-$1 ] || rm -rf DataSeriesExamples-$1
    tar xvvfz DataSeriesExamples-$1.tar.gz
}

buildOld() {
    case $BUILD_OS in
        centos-5.*) : ;; # no boost_foreach, can't work.
        ubuntu-8.04*) : ;; # old cmake, no boost::regex
        *) cd DataSeriesExamples-$OLD_VERSION/triangle-find;
            cmake . && make && ctest
            cd ../simple
            make
            cd $START_DIR
            ;;
    esac
}

buildNew() {
    cd DataSeriesExamples-$1/triangle-find
    case $BUILD_OS in
        centos-5.*) cd ../simple && make && make test ;; # no boost_foreach, can't work.
        ubuntu-8.04*) : ;; # too old, no tests work.
        *) cmake . && make && ctest
           cd ../simple && make && make test
            ;; 
    esac
    cd $START_DIR
}

downloadUntar $OLD_VERSION
downloadUntar $RELEASE_VERSION
echo "-------------------------------------------------------------"
echo "Test Example Build $RELEASE_VERSION / $BUILD_OS"
echo "-------------------------------------------------------------"
buildOld
buildNew $RELEASE_VERSION


