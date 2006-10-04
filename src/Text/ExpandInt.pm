package Text::ExpandInt;
#
#  (c) Copyright 2004-2006, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
#  Module to expand out integers into an collection of arguments

use strict;
use warnings;
require Exporter;

use vars qw/@EXPORT_OK @ISA/;
@ISA = qw/Exporter/;
@EXPORT_OK = qw/expand/;

sub expand {
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

Text::ExpandInt - a function to expand out multiple arguments

=head1 SYNOPSIS

use Text::ExpandInt 'expand';

my @list = expand("foo[1,7,11]-[1-5%2,9-13%2]");

=head1 DESCRIPTION

my @list = expand("foo[1,7,11]-[1-5%2,9-13%2]");

# Returns: foo1-1, foo1-3, foo1-5, foo1-9, foo1-11, foo1-13, foo7-1,
# foo7-3, foo7-5, foo7-9, foo7-11, foo7-13, foo11-1, foo11-3, foo11-5,
# foo11-9, foo11-11, foo11-13

# Expandint will expand out the bits inside of [] expanding out so
# that the later [] bits are expanded out first.

# The structure inside a single [] block is a list of , separated
# entries.  Each entry is either a single integer (#), a range of
# integers (#-#), or a range of integers with a step (#-#%#).

