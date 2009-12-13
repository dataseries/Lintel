#!/usr/bin/perl

use Lintel;

my $stat = new Lintel::StatsQuantile;

$stat->add(4);
$stat->add(2);

die "?" unless $stat->mean() == 3;

die "?" unless $stat->getQuantile(0.45) == 2;
die "?" unless $stat->getQuantile(0.55) == 4;

exit(0);
