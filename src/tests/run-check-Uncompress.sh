#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# run the check-Uncompress test 

set -e

./check-Uncompress $1/../Uncompress.cpp | diff -c $1/../Uncompress.cpp -
gzip -1cv <$1/../Uncompress.cpp >Uncompress.cpp.gz
./check-Uncompress Uncompress.cpp.gz | diff -c $1/../Uncompress.cpp -
rm Uncompress.cpp.gz

exit 0

