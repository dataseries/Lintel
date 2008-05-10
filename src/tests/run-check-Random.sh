#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# run the check-Random test 

set -e
./check-Random 2>&1 | diff -b $1/Random.`uname`.regression -
exit 0
