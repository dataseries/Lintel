#!/bin/sh
set -e

./testutil >testutil.out 2>&1 || true

sed 's/Assertion failure in file .*testutil.cpp, line.*/Assertion failure in testutil.cpp/' < testutil.out | grep -v '^Aborted$' >testutil.check

cat >testutil.good <<EOF 

**** Assertion failure in testutil.cpp
**** Failed expression: e.msg == "wrong test msg"
**** Details: unexpected error message 'expected to fail'
EOF

cmp testutil.good testutil.check
rm testutil.out testutil.check testutil.good

