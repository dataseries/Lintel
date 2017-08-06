#!/bin/bash
#set -x
# Build+Install Lintel (debugging version)
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
    test $RUNCMD_SLEEP != 0  &&  sleep $RUNCMD_SLEEP
    "$@"
    ret=$?
    if test $ret -ne 0 ; then
	exit $ret
    fi
}

if ! test -d src/Lintel ; then
    echo "no subdir src/lintel. Please cd to top level Lintel source repo."
    exit 1
fi
TOP=$PWD

# clean old install dirs
runcmd rm -fr $TARGET/dataseries.old
if test -d $TARGET/dataseries ; then
    runcmd mv $TARGET/dataseries $TARGET/dataseries.old
fi
export PATH=$TARGET/dataseries/bin:$PATH

runcmd mkdir -p build

# configure and build Lintel
runcmd cd $TOP/build
runcmd cmake \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    -DBUILD_SHARED_LIBS=ON \
    -DWITH_PERL=ON \
    -DCMAKE_CXX_FLAGS_RELWITHDEBINFO='-fPIC -O2 -g -DNDEBUG -pg' \
    -DCMAKE_INSTALL_PREFIX:PATH=$TARGET/dataseries \
    $TOP
runcmd make -j$PARALLELISM
runcmd make install

# Note: DS Makefiles have some dependency that breaks "gmake -j8", so
# use -j2 or none
echo "CMD: make -j$PARALLELISM"
runcmd make -j$PARALLELISM
runcmd make install
#runcmd rm -fr $TOP/build
