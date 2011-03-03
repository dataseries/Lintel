#!/bin/sh
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
LINTEL_PO_TEST=sixth ./program_options --po-1 --test-opt-1 --po-2=5 --test-opt-4=5 || exit 1
LINTEL_PO_TEST=stream_read ./program_options --mode=stream_read --file-path=$SRCDIR/program-options.txt || exit 1
LINTEL_PO_TEST=file_read ./program_options --mode=file_read --file-path=$SRCDIR/program-options.txt || exit 1

output_file="/tmp/`whoami`.po_help_width";
# 41 is minimal width because of program option name (it results in a single column of text on
# boost1.35, run up to 250 with a variety of sizes, 80, 100, 120 are "expected" values.  Would like
# to use random sizes, but that isn't in POSIX shells.  Interestingly, boost 1.35 will loop
# indefinitely for a help_width of 40.
for help_width in 41 80 100 113 120 140 159 181 211 231 250; do
    echo "Testing with help-width = $help_width"
    # Prune out blank lines
    COLUMNS=$help_width LINTEL_PO_TEST=help-width-test ./program_options --help | sed '/^$/d' >| $output_file || exit 1
    num_of_lines=`wc -l < $output_file`
    max=`expr $num_of_lines - 1` # last line can have arbitrary size.
    for j in `seq 4 $max`; do
        length=`head -$j $output_file | tail -1 | wc -c`
        # boost prints description as paragraph and does not chop words if they start in second
        # half of line. Max word length in po description is 20.
        lower_bound=`expr $help_width - 20`;
        # check all lengths in [$lower_bound .. $help_width], ignoring blank lines
        if [ "$length" -gt "$help_width" -o "$length" -lt "$lower_bound" ]; then
            echo "Test failed " $length $help_width;
            exit 1
        fi
        echo "Test passed for" $i $j $length $help_width;
    done
done
rm $output_file
