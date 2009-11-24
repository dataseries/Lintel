#!/bin/sh
set -e
grep -v 'BEGIN.*INC.*build/opt' deptool | grep -v 'use Lintel' >deptool-bootstrap.tmp
# If you update the next line, you should update the dependencies in src/CMakeLists.txt
cat $1/Lintel/File/Lock.pm $1/Lintel/ProcessManager.pm >>deptool-bootstrap.tmp
mv deptool-bootstrap.tmp deptool-bootstrap
