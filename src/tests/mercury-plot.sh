#!/bin/sh
set -e
echo quit >tmp.hg
../mercury-plot <tmp.hg
../mercury-plot tmp.hg
../mercury-plot --man


