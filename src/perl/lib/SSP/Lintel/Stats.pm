package SSP::Lintel::Stats;

sub new {
    my($class) = @_;
    return bless {'count' => 0, 
		  'sum' => 0,
		  'sumsq' => 0,
		  'max' => -1e100,
		  'min' => 1e100}, $class;
}

sub add {
    $_[0]->{count} += 1;
    $_[0]->{sum} += $_[1];
    $_[0]->{max} = $_[1] if $_[1] > $_[0]->{max};
    $_[0]->{min} = $_[1] if $_[1] < $_[0]->{min};
    $_[0]->{sumsq} += $_[1] * $_[1];
}

sub mean {
    return 'NaN' if $_[0]->{count} == 0;
    return $_[0]->{sum} / $_[0]->{count};
}

sub stddev {
    return 'NaN' if $_[0]->{count} == 0;
    my $mean = $_[0]->mean();
    my $variance = $_[0]->{sumsq} / $_[0]->{count} - $mean * $mean;
    return sqrt($variance);
}

1;
