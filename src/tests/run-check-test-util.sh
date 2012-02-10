#!/bin/sh
set -e -x

./testutil >testutil.out 2>&1 || true

sed 's/Assertion failure in file .*testutil.cpp, line.*/Assertion failure in testutil.cpp/' < testutil.out | grep -v '^Aborted' >testutil.check

cat >testutil.good <<EOF 

**** Assertion failure in testutil.cpp
**** Failed expression: found
**** Details: unexpected error message 'expected to fail'
EOF

if [ "`uname -s`" = OpenBSD ]; then
    echo "Abort trap " >>testutil.good
fi

cmp testutil.good testutil.check
rm testutil.out testutil.check testutil.good

