#!/bin/sh
set -e
set -x
HOST_ADDR=192.168.122.1
no_proxy=$HOST_ADDR
export no_proxy
# freebsd symlinks /home to /usr/home; so PWD is confusing.  cd to $HOME to fix
cd $HOME
[ ! -f deptool-bootstrap ] || rm deptool-bootstrap
[ ! -d build ] || rm -rf build
[ ! -d projects ] || rm -rf projects
wget --no-cache http://$HOST_ADDR/pb-sources/deptool-bootstrap
perl deptool-bootstrap init --tar --no-cache http://$HOST_ADDR/pb-sources/latest-release
cd projects/DataSeries
perl ../../deptool-bootstrap build -t
