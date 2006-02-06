#
#  (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

package BatchParallel::compress;

die "module version mismatch" 
    unless $BatchParallel::common::interface_version < 2;

@ISA = qw/BatchParallel::common/;

sub usage {
    print <<END_OF_USAGE;
batch-parallel compress [noclean] [nocheck] [gz] [bz2] [level=#]
  defaults are bz2 compression at level 9 with both cleaning (removing 
  the source file), and checking (re-uncompressing and comparing data)
END_OF_USAGE
}

sub new {
    my $class = shift;
    
    my $this = { 'clean' => 1, 'check' => 1, 'type' => 'bz2', 'level' => 9 };
    foreach my $arg (@_) {
	if ($arg eq 'noclean') {
	    $this->{clean} = 0;
	} elsif ($arg eq 'nocheck') { 
	    $this->{check} = 0;
	} elsif ($arg eq 'gz') {
	    $this->{type} = 'gz';
	} elsif ($arg eq 'bz2') {
	    $this->{type} = 'bz2';
	} elsif ($arg =~ /^level=(\d+)$/) {
	    $this->{level} = $1;
	} elsif ($arg eq 'help') {
	    usage();
	    exit(0);
	} else {
	    usage();
	    die "unable to interpret argument '$arg'."
	}
    }
    print "compressing to $this->{type} at level $this->{level}, ", 
        ($this->{check} ? "checking" : "not checking"), ", ", 
        ($this->{clean} ? "cleaning" : "not cleaning"), "\n";
    return bless $this, $class;
}

sub file_is_source {
    my($this,$prefix,$fullpath,$filename) = @_;

    return 0 if -l $filename;
    return 0 unless -f $filename;
    return 0 if $filename =~ /\.$this->{type}$/o;
    return 1;
}

sub destination_file {
    my($this,$prefix,$fullpath) = @_;

    while($fullpath =~ s/\.((gz)|(bz2))$//o) { 
        # chop off all of the previous compression bits
    }
    return $fullpath . ".$this->{type}";
}

sub destfile_out_of_date {
    my($this,$prefix,$fullpath,$destfile) = @_;

    return 0 unless -f $destfile;
    return 1;
}

sub fail {
    my $destpath = shift;

    unlink $destpath;
    die @_;
}

sub rebuild {
    my($this,$prefix,$fullpath,$destpath) = @_;

    my @cmds = ("cat $fullpath");
    my $origpath = $fullpath;
    while(1) {
	if ($fullpath =~ s/\.gz$//o) {
	    push(@cmds,"gunzip");
	} elsif ($fullpath =~ s/\.bz2$//o) {
	    push(@cmds,"bunzip2");
	} else {
	    last;
	}
    }
    my $unpack_orig_cmd = join("|",@cmds);
    my $pack_cmd;
    my $unpack_cmd;

    if ($this->{type} eq 'bz2') {
	$pack_cmd = "bzip2 -v -$this->{level}";
	$unpack_cmd = "bunzip2";
    } elsif ($this->{type} eq 'gz') {
	$pack_cmd = "gzip -v -$this->{level}";
	$unpack_cmd = "gunzip";
    }
    my $ret = system("$unpack_orig_cmd | $pack_cmd >$destpath");
    unless($ret == 0) {
	print "$unpack_orig_cmd | $pack_cmd >$destpath failed";
	unlink($destpath);
	return 0;
    }
    if ($this->{check}) {
	open(F1,"$unpack_orig_cmd |") || fail($destpath,"can't run $unpack_orig_cmd: $!");
	open(F2,"$unpack_cmd < $destpath |") || fail($destpath,"can't run $unpack_cmd < $destpath: $!");
	my $bufsize = 64*1024;
	my($f1,$f2);
	while(1) {
	    my $amt1 = read(F1,$f1,$bufsize);
	    my $amt2 = read(F2,$f2,$bufsize);
	    fail($destpath,"read failed") unless defined $amt1 && defined $amt2;
	    fail($destpath,"different sizes $amt1 != $amt2") unless $amt1 == $amt2;
	    last if $amt1 == 0;
	    fail($destpath,"read data different") unless $f1 eq $f2;
	}
    }
    if ($this->{clean}) {
	unlink($origpath);
    }
    return 1;
}
	    
1;

    
