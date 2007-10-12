package Plot::Mercury::Tics;
use strict;

# relative_hourunits is preserved because of
# DataSeries/src/analysis/lsfdsplots if we can ditch it there, then we
# can clean up this code; using a non-seconds unit was almost
# assuredly a mistake because early versions of that code wanted to
# just generate xtics easily.

sub plottics {
    my($starttime, $endtime, $relative_hourunits) = @_;

    my $tickstep;
    my $nsecs = $endtime - $starttime;
    my $nhours = $nsecs / 3600;
#    print "NHOURS = $nhours NSECS=$nsecs\n";
    # the sub-1 hours tick steppings are not yet fully worked out.
    # basically this code is trying to balance the number of tics that we get
    # with the range we are covering and provide tics that are "aligned" 
    # to human-expected points.
    if ($nsecs <= 300) {
	$tickstep = 30;
    } elsif ($nsecs <= 600) {
	$tickstep = 60;
    } elsif ($nhours <= 1) {
	$tickstep = 180;
    } elsif ($nhours <= 3) {
	$tickstep = 600;
    } elsif ($nhours <= 6) {
	$tickstep = 1800;
    } elsif ($nhours <= 12) {
	$tickstep = 1 * 3600;
    } elsif ($nhours <= 24) { 
	$tickstep = 2 * 3600;
    } elsif ($nhours <= 48) {
	$tickstep = 4 * 3600;
    } elsif ($nhours <= 6*12) {
	$tickstep = 6 * 3600;
    } elsif ($nhours <= 8*12) {
	$tickstep = 8 * 3600;
    } elsif ($nhours <= 12*12) {
	$tickstep = 12 * 3600;
    } elsif ($nhours < 24*12) {
	$tickstep = 24 * 3600;
    } else {
	return plottics_large($starttime,$endtime,$relative_hourunits);
    }

    my $lmin = 48*3600 * int($starttime / (48*3600)) - 24*3600;
    while(1) {
	my @t = localtime($lmin);
	last if $t[2] == 0;
	$lmin += 3600;
    }

    while ($lmin < $starttime) {
	$lmin += $tickstep;
    }
    my @ret;
    for(;$lmin < $endtime;$lmin += $tickstep) {
        my @t = localtime($lmin);
	my $i = ($lmin - $starttime)/3600;
	if ($tickstep >= 24*3600) { # handle possible crossing of DST
	    if ($t[2] == 1) { 
		my $tmin = $lmin - 3600;
		@t = localtime($tmin);
		$i = ($tmin - $starttime)/3600;
	    } elsif ($t[2] == 23) {
		my $tmin = $lmin + 3600;
		@t = localtime($tmin);
		$i = ($tmin - $starttime)/3600;
	    } elsif ($t[2] == 0) {
		# already ok
	    } else {
		warn "Can't correct for DST crossing ?! $t[2] $lmin $tickstep";
	    }
#	    die "weird no DST correction $t[2] $lmin $i" unless $t[2] == 0;
	}
	my $label;
	if ($t[2] == 0) {
	    die "??" unless $t[1] == 0 && $t[0] == 0;
	    my $x = sprintf("%4d-%02d-%02d",$t[5]+1900,$t[4]+1,$t[3]);
	    $label = $x;
        } elsif ($t[0] == 0) {
	    die "?? $t[0]" unless $t[0] == 0;
	    $label = "$t[2]:$t[1]";
        } else {
	    $label = "$t[2]:$t[1]:$t[0]";
	}
	if ($relative_hourunits) {
	    push(@ret,"\"$label\" $i");
	} else {
	    push(@ret,"\"$label\" $lmin");
	}
    }
    return "set xtics (" . join(", ",@ret) . ")";
}

sub plottics_large {
    my($starttime,$endtime, $relative_hourunits) = @_;
    my $ndays = ($endtime - $starttime) / (24*3600);
    my $daymin = 3600*int($starttime/3600);
    while ((localtime($daymin))[2] != 0) {
	$daymin += 3600;
    }
#     print "XX $daymin", scalar localtime($daymin), "\n";
    my @ret;
    if ($ndays <= 12 * 7) {
	# ticks by week
	my $wday = (localtime($daymin))[6];
	while($daymin < $endtime) {
	    my @t = localtime($daymin);
	    my $x = sprintf("%4d-%02d-%02d",$t[5]+1900,$t[4]+1,$t[3]);
	    my $i = ($daymin - $starttime)/3600;
	    if ($relative_hourunits) {
		push(@ret,"\"$x\" $i");
	    } else {
		push(@ret,"\"$x\" $daymin");
	    }
	    $daymin += 86400*6; # advance to close to same day next week
	    while((localtime($daymin))[6] != $wday) {
		$daymin += 3600;
	    }
	}
    } else {
	# ticks by month
	my $wday = (localtime($daymin))[3];
	my $prev_i = -1000000;
	my $minstep = ($endtime - $starttime) / (3600*11);
	print "XXXX $minstep\n";
	while($daymin <= $endtime) {
	    my @t = localtime($daymin);
	    my $x = sprintf("%4d-%02d-%02d",$t[5]+1900,$t[4]+1,$t[3]);
	    my $i = ($daymin - $starttime)/3600;
  	    if (($i - $prev_i) > $minstep) {
		if ($relative_hourunits) {
		    push(@ret,"\"$x\" $i");
		} else {
		    push(@ret,"\"$x\" $daymin");
		}
		$prev_i = $i;
	    }
	    $daymin += 86400*26; # advance to close the same day next month
	    while((localtime($daymin))[3] != $wday) {
		$daymin += 3600;
	    }
	}
	pop @ret;
	my @t = localtime($endtime);
	my $x = sprintf("%4d-%02d-%02d",$t[5]+1900,$t[4]+1,$t[3]);
	my $i = ($endtime - $starttime)/3600;
	if ($relative_hourunits) {
	    push(@ret,"\"$x\" $i");
	} else {
	    push(@ret,"\"$x\" $endtime");
	}
    }
    return "set xtics (" . join(", ",@ret) . ")";
}

1;
