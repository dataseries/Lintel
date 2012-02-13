#!/bin/sh
error() {
    echo "Error checking web-sites.bib: $@"
    exit 1
}
BIB=`dirname $0`/web-sites.bib
echo "Checking $BIB for errors, see README for description of rules"
[ -f $BIB ] || error "Input file $BIB is not present"

AT_COUNT=`grep '^\@' $BIB | wc -l | sed 's/^ *//'`
MISC_COUNT=`grep '^@misc{' $BIB | wc -l | sed 's/^ *//'`
KEY_COUNT=`grep '^  key ' $BIB | wc -l | sed 's/^ *//'`
AUTHOR_COUNT=`grep '^  author ' $BIB | wc -l | sed 's/^ *//'`
NOTE_COUNT=`grep '^  note ' $BIB | wc -l | sed 's/^ *//'`
NOTE_URL_COUNT=`grep '^  note.*url' $BIB | wc -l | sed 's/^ *//'`
NOTE_ACCESSED_COUNT=`grep '^  note.*Accessed.*20[0-9][0-9]' $BIB | wc -l | sed 's/^ *//'`

[ "$AT_COUNT" -ge 5 ] || error "Have fewer than 5 references?"
[ "$AT_COUNT" = "$MISC_COUNT" ] || error "Mismatch between stanzas and misc stanzas: $AT_COUNT != $MISC_COUNT"
KEY_AUTHOR_SUM=`expr $KEY_COUNT + $AUTHOR_COUNT`
[ "$KEY_AUTHOR_SUM" = "$MISC_COUNT" ] || error "Mismatch between # key/author entries and # references: $KEY_COUNT + $AUTHOR_COUNT != $MISC_COUNT"
[ "$NOTE_COUNT" = "$MISC_COUNT" ] || error "Mismatch between # note entries and # references: $NOTE_COUNT != $MISC_COUNT"
[ "$NOTE_URL_COUNT" = "$MISC_COUNT" ] || error "Mismatch between # url's in note entries and # references: $NOTE_URL_COUNT != $MISC_COUNT"
[ "$NOTE_ACCESSED_COUNT" = "$MISC_COUNT" ] || error "Mismatch between # note entries with Accessed info and # references: $NOTE_ACCESSED_COUNT != $MISC_COUNT"
echo "check passed."
exit 0
