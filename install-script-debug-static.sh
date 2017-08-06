#!/bin/bash
#set -x
# Install Lintel and DataSeries (debugging version)
#
# Builds in $CWD/build; installs in $TARGET/dataseries (default target: $HOME)
TARGET=${TARGET:-$HOME}

# Amount of parallelism to ask of make
PARALLELISM=${PARALLELISM:-8}

# How much time to waste on each command
RUNCMD_SLEEP=${RUNCMD_SLEEP:-0.2}

function runcmd
{
    echo "CMD: $@"
    (( $RUNCMD_SLEEP != 0 ))  &&  sleep $RUNCMD_SLEEP
    "$@"
    ret=$?
    if test $ret -ne 0 ; then
	exit $ret
    fi
}

if ! test -d projects/DataSeries ; then
    echo "no subdir projects/DataSeries. Please cd to top level dataseries source repo."
    exit 1
fi
TOP=$PWD

# clean old install dirs
runcmd rm -fr $TARGET/dataseries.old
if test -d $TARGET/dataseries ; then
    runcmd mv $TARGET/dataseries $TARGET/dataseries.old
fi
export PATH=$TARGET/dataseries/bin:$PATH

# clean old build dirs
#runcmd rm -fr build.old
#if test -d build ; then
#    runcmd mv build build.old
#fi
runcmd mkdir -p build/Lintel build/DataSeries

# configure and build Lintel
runcmd cd $TOP/build/Lintel
runcmd cmake \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DWITH_PERL=ON \
    -DCMAKE_CXX_FLAGS_RELWITHDEBINFO='-fPIC -O2 -g -DNDEBUG -pg' \
    -DCMAKE_INSTALL_PREFIX:PATH=$TARGET/dataseries \
    $TOP/projects/Lintel
runcmd make -j$PARALLELISM
runcmd make install

# configure and build DataSeries
runcmd cd $TOP/build/DataSeries
runcmd cmake \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_CXX_FLAGS_RELWITHDEBINFO='-fPIC -O2 -g -DNDEBUG -pg' \
    -DCMAKE_INSTALL_PREFIX:PATH=$TARGET/dataseries \
    $TOP/projects/DataSeries

# Note: DS Makefiles have some dependency that breaks "gmake -j8", so
# use -j2 or none
echo "CMD: make -j$PARALLELISM"
runcmd make -j$PARALLELISM
runcmd make install
#runcmd rm -fr $TOP/build

# useful info from cmake -i (interactive run to see all flags)
#cmake -DBUILD_SHARED_LIBS=OFF
#Variable Name: CMAKE_BUILD_TYPE
#Description: Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.
#CMAKE_C_FLAGS?
# what does "Debug" build use?
# cmake -DCMAKE_BUILD_TYPE=Debug
#CMAKE_CXX_FLAGS (empty)
#CMAKE_CXX_FLAGS_DEBUG (-g)
#CMAKE_CXX_FLAGS_RELWITHDEBINFO (-O2 -g -DNDEBUG)
#ENABLE_CLOCK_TEST=ON -- what does it do?

