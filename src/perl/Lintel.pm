#
#  (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#


package Lintel;

use strict;
use Carp;
use vars qw($VERSION);

require DynaLoader;

$VERSION = '0.01';

@LintelPerl::ISA = qw(DynaLoader);

bootstrap LintelPerl $VERSION;

@Lintel::Histogram::ISA = qw(Lintel::Stats);
@Lintel::Histogram::Uniform::ISA = qw(Lintel::Histogram);
@Lintel::Histogram::Log::ISA = qw(Lintel::Histogram);
@Lintel::StatsQuantile::ISA = qw(Lintel::Stats);

1;
__END__

=head1 NAME

Lintel - Perl extension for accessing a subset of the Lintel functionality

=head1 SYNOPSIS

  use Lintel;

  my $stat = new Lintel::Stats;
  $stat->add(I<val>);
  my ($count, $min, $max) = ($stat->count(), $stat->min(), $stat->max());
  my ($mean, $stddev, $conf95) = ($stat->mean(), $stat->stddev(), $stat->conf95());

  my $qstat = new Lintel:StatsQuantile($error = 0.01, $nbound = 1.0e9)
  $qstat->add(I<val>); # and other Lintel::Stats methods...
  my $quantile = $qstat->getQuantile(0.1); # 0..1

=head1 DESCRIPTION

Simple access to some of the Lintel functionality.  See man Stats, StatsQuantile
for details.

=head1 AUTHOR

Eric Anderson, eric.anderson4@hp.com

=head1 SEE ALSO

perl(1), Stats(3), StatsQuantile(3).

=cut
