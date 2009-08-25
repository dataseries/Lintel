#!/bin/sh -x
SRCDIR=$1
[ -d "$SRCDIR" ] || exit 1
LINTEL_PO_TEST=first ./program_options --mode=first --test --multi=1 --multi=2 || exit 1
LINTEL_PO_TEST=second ./program_options --sample=a --unknown foo || exit 1
LINTEL_PO_TEST=third ./program_options --help >check-help || exit 1
cmp check-help $SRCDIR/program_options.help.ref || exit 1
LINTEL_PO_TEST=fourth ./program_options -x || exit 1
LINTEL_PO_TEST=fifth ./program_options  --cmdline1 --cmdline2=3 --cmdline3=8 --cmdline3=8 --cmdline3=8 --cmdline3=8 || exit 1
# TODO-joe: since part of the point of test options is they don't show up
# in the help output, you need to check that too, similar to how test third
# works, but just extending it isn't sufficient since you need to make sure
# the options are parsed.
LINTEL_PO_TEST=sixth ./program_options --po-1 --test-opt-1 --po-2=5 --test-opt-4=5 --no-po-1 || exit 1


