#!/bin/sh
(
echo ">>>>Expect Assert to fail on 2nd call"
echo LintelAssert
./check-LintelAssert
echo "==============================================================="
echo ">>>>Expect Assert to fail on 1st call"
echo LintelAssert 99
./check-LintelAssert 99
echo "==============================================================="
echo ">>>>Expect Assert to fail on recursion fault"
echo LintelAssert -R
./check-LintelAssert -R
echo "==============================================================="
echo ">>>>Expect Assert to fail on fatal fault"
echo LintelAssert -F
./check-LintelAssert -F
) 2>&1 | grep -v './run-check-LintelAssert.sh' | grep -v 'Aborted$' | sed "s/.cpp, [0-9]*:/.cpp, LINENO:/" | sed 's,Assert failure in .*/tests/check-Lintel,Assert failure in tests/check-Lintel,' >./tmp-LintelAssert.out
set -e
echo "diff -c $1/./LintelAssert.good ./tmp-LintelAssert.out"
diff -c $1/./LintelAssert.good ./tmp-LintelAssert.out
rm ./tmp-LintelAssert.out

