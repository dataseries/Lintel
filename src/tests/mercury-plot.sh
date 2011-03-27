#!/bin/sh
set -e
echo quit >tmp.hg
export PERL5LIB=$1:$PERL5LIB
../mercury-plot <tmp.hg
../mercury-plot tmp.hg
../mercury-plot --man


