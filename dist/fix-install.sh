#!/bin/sh
set -e -x
[ -d "$1" ]
TMP_USR=$1/usr

perl -ne 's,% (.+)/Lintel,% .../Lintel,; print;' -i $TMP_USR/share/lintel-latex-rebuild/lintel-latex-bib.bib
perl -ne 's,% (.+)/Lintel,% .../Lintel,; print;' -i $TMP_USR/share/lintel-latex-rebuild/lintel-latex-all.bib
# Unclear whether lexgrog or doxygen is wrong, but lexgrog does not accept spaces 
# on the left hand side of the NAME entry.
find $TMP_USR/share/man/man3 -type f -print0 | xargs -0 perl -i -p -e 'if ($fix) { ($left, $right) = /^(.+)( \\-.+)$/; $left =~ s/ //go; $_ = "$left$right\n"; $fix = 0; }; $fix = 1 if /^.SH NAME/o;' 
for i in $TMP_USR/bin/drawRandomLogNormal $TMP_USR/lib/libLintel.so.*.* $TMP_USR/lib/libLintelPThread.so.*.* $TMP_USR/lib/perl5/libLintelPerl.so; do
    echo "Removing rpath from $i"
    chrpath -d $i
done

