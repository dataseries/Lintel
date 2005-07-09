#!/bin/sh
(
echo ">>>>Expect Assert to fail on 2nd call"
echo LintelAssert
tests/check-LintelAssert
echo "==============================================================="
echo ">>>>Expect Assert to fail on 1st call"
echo LintelAssert 99
tests/check-LintelAssert 99
echo "==============================================================="
echo ">>>>Expect Assert to fail on recursion fault"
echo LintelAssert -R
tests/check-LintelAssert -R
echo "==============================================================="
echo ">>>>Expect Assert to fail on fatal fault"
echo LintelAssert -F
tests/check-LintelAssert -F
) 2>&1 | grep -v 'tests/run-check-LintelAssert.sh' | sed "s/.C, [0-9]*:/.C, LINENO:/" | sed 's,Assert failure in .*tests.check-Lintel,Assert failure in tests/check-Lintel,' >tests/tmp-LintelAssert.out
set -e
echo "diff -c $1/tests/LintelAssert.good tests/tmp-LintelAssert.out"
diff -c $1/tests/LintelAssert.good tests/tmp-LintelAssert.out
rm tests/tmp-LintelAssert.out

