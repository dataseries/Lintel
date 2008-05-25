#!/bin/sh
set -e

LINTEL_LOG_DEBUG=env ./lintel_log >log_out 2>log_err

cat >log_out.correct <<EOF
report string
second report string line
DEBUG: debug string
INFO: info string
report 5
line2 happy
DEBUG: debug format
INFO: info format
LintelLog tests successful
EOF

TIME=`grep 'close to running' log_err| sed -e 's/:.*$//' -e 's/^WARN .//'`

cat >log_err.correct <<EOF
WARN: warn string
ERROR: error string
WARN: warn format
ERROR: error format
WARN @$TIME: close to running out of reserved categories 4800/6000 used; recommend calling LintelLog::reserveCategories(9000) or greater
EOF

cmp log_out log_out.correct
cmp log_err log_err.correct

rm log_out log_err log_out.correct log_err.correct
