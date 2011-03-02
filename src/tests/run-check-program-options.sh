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
LINTEL_PO_TEST=sixth ./program_options --po-1 --test-opt-1 --po-2=5 --test-opt-4=5 || exit 1
LINTEL_PO_TEST=stream_read ./program_options --mode=stream_read --file-path=$SRCDIR/program-options.txt || exit 1
LINTEL_PO_TEST=file_read ./program_options --mode=file_read --file-path=$SRCDIR/program-options.txt || exit 1

output_file="/tmp/`whoami`.po_help_width";
for ((i=0; i<10; ++i))
  do
  let "help_width = 40 + ($RANDOM % 200)"; # 40 is width program option name
  LINTEL_PO_HELP_WIDTH=$help_width LINTEL_PO_TEST=help-width-test ./program_options --help >| $output_file || exit 1
  num_of_lines=$(cat $output_file | wc -l)
  echo $num_of_lines
  for ((j=4; j<$num_of_lines-1; j++))
    do
    length=$(head -$j $output_file | tail -1 | wc -c)
    lower_bound=`expr $help_width - 20`; # 20 is max length word
    if [ "$length" -gt "$help_width" -o "$length" -lt "$lower_bound" -a "$length" -ne "0" ]
        then
        echo "Test failed " $length $help_width;
        exit 1;
    fi
    echo "Test passed for" $i $j $length $help_width;
  done
done
