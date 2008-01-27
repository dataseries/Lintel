package Text::Expand;
#
#  (c) Copyright 2004-2007, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
#  Module to expand out integers into an collection of arguments

use strict;
use warnings;
require Exporter;

use vars qw/@EXPORT_OK @ISA/;
@ISA = qw/Exporter/;
@EXPORT_OK = qw/expandString/;

sub expandString {
    my @ret;
    my @pending = @_;
    while (@pending > 0) {
	my $thing = shift @pending;
	if ($thing =~ /^(.*?)(\[([0-9,\-\%]+)?\])(.*)$/o) {
	    my($prefix,$full,$nums,$suffix) = ($1,$2,$3,$4,$5);
	    my @bits = split(/,/o,$nums);
	    foreach my $bit (@bits) {
		if ($bit =~ /^\d+$/o) {
		    push(@pending,"$prefix$bit$suffix");
		} elsif ($bit =~ /^(\d+)-(\d+)(?:\%(\d+))?$/o) {
		    my ($min,$max,$step) = ($1,$2,$3);
		    $step = 1 unless defined $step;
		    die "$min > $max" if $min > $max;
		    my $printfspec = "%s%d%s";
		    $printfspec = "%s%0" . (length $min) . "d%s"
			if length $min eq length $max;
		    $step = 1 unless defined $step;
		    die "step doesn't match properly with $min .. $max"
			unless ($max - $min) % $step == 0;
		    for(my $i=$min; $i <= $max; $i += $step) {
			push(@pending,sprintf $printfspec, $prefix,$i,$suffix);
		    }
		} else {
		    die "Unable to interpret $bit";
		}
	    }
	} else {
	    push(@ret,$thing);
	}
    }
    return @ret;
}

1;

__END__

=head1 NAME

Text::Expand - a function to expand out multiple arguments

=head1 SYNOPSIS

use Text::Expand 'expandString';

    my @list = expandString("a[0-3]"); # qw(a0 a1 a2 a3)
    @list = expandString("[0,2,4]b"); # qw(0b 2b 4b)
    @list = expandString("a[0,2,3-5]c"); # qw(a0c a2c a3c a4c a5c)
    @list = expandString("d[0-12%2]"); # qw(d0 d2 d4 d6 d8 d10 d12)
    @list = expandString("e[00-12%2]"); # qw(d00 d02 d04 d06 d08 d10 d12)
    @list = expandString(""f[0,1]g[03-05]"); 
      # qw(f0g03 f0g04 f0g05 f1g03 f1g04 f1g05)
    @list = expandString("foo[1,7,11]-[1-5%2,9-13%2]");
      # see description

=head1 DESCRIPTION

my @list = expandString("foo[1,7,11]-[1-5%2,9-13%2]");

# Returns: foo1-1, foo1-3, foo1-5, foo1-9, foo1-11, foo1-13, foo7-1,
# foo7-3, foo7-5, foo7-9, foo7-11, foo7-13, foo11-1, foo11-3, foo11-5,
# foo11-9, foo11-11, foo11-13

# Text::Expand will expand out the bits inside of [] expanding out so
# that the later [] bits are expanded out first.

# The structure inside a single [] block is a list of , separated
# entries.  Each entry is either a single integer (#), a range of
# integers (#-#), or a range of integers with a step (#-#%#).

=head1 TODO

It would be nice if we could specify non-integer constructs for ,
separated expansion, and possibly for constructs like a-z for -
expansion, although it is not exactly clear how aa-zz should be
interpreted, e.g. as aa,bb,cc,dd, ..., zz; or as aa, ab, ac, ..., az,
ba, ..., zz. Could start with the unambiguous options of , separated 
strings.


