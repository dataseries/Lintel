package BatchParallel::jobsfile;

use POSIX;

die "module version mismatch" 
    unless $BatchParallel::common::interface_version < 2;

@ISA = qw/BatchParallel::common/;

sub usage {
    print <<END_OF_USAGE;
batch-parallel jobsfile [printcmd] -- file output-dir
  printcmd: include command in output-dir/<line#>

  - The file should specify the command to execute 1 job/line.
  - Lines with a # at the front of them will be ignored (but counted
    in the line count)
  - The output directory will be created if it doesn't already exist.
  - Stdout and stderr for each job will be placed in output-dir/<line#>
  - Jobs that return an error, or fail to complete because the host crashes
    will have their output in output-dir/<line#>-hostname-pid
  - if a line# file already exists, the job will not be re-executed
  - line#'s count starting from 1
END_OF_USAGE
}

sub new {
    my $class = shift;
    
    my $this = { };
    foreach my $arg (@_) {
	if ($arg eq 'help') {
	    usage();
	    exit(0);
	} elsif ($arg eq 'printcmd') {
	    $this->{printcommand} = 1;
	} else {
	    usage();
	    die "unable to interpret argument '$arg'."
	}
    }
    return bless $this, $class;
}

sub find_things_to_build {
    my($this,@dirs) = @_;

    unless (@dirs == 2) {
	usage();
	die "incorrect number of arguments";
    }
    my ($joblist,$outdir) = @dirs;
    
    $|=1;
    print "Reading in input jobs...";
    open(JOBSLIST,$joblist) 
	or die "Can't open $joblist for read: $!";
    unless(-d $outdir) {
	mkdir($outdir,0755) or die "can't mkdir $outdir: $!";
    }
    my $linenum = 0;
    my @possibles;
    while(<JOBSLIST>) {
	++$linenum;
	next if /^#/o;
	s/[\r\n]$//o;
	push(@possibles,[$linenum,$_]);
    }
    close(JOBSLIST) or die "close failed: $!";
    print "done.\n";
    my $outfile_fmt = "%0" . POSIX::ceil(1+log($linenum)/log(10)) . "d";
    
    print "checking for existing jobs...";
    my @jobs;
    for(my $i = 0; $i < @possibles; ++$i) {
	print "." if ($i % 100) == 0;
	my ($linenum,$cmd) = @{$possibles[$i]};
	my $outfile = sprintf("%s/$outfile_fmt",$outdir,$linenum);
	next if -f $outfile;
	push(@jobs,[$outfile,$cmd]);
    }
    return ($linenum,@jobs);
}

sub rebuild_thing_message {
    my($this,$thing_info) = @_;

    my($outfile,$cmd) = @$thing_info;

    print "should run $cmd with output to $outfile\n";
}

# you will be forked before this is called; you should call exit to
# complete the function, it is an error to return.

sub rebuild_thing_do { 
    my($this,$thing_info) = @_;

    my($outfile,$cmd) = @$thing_info;

    my $hostname = `hostname`;chomp($hostname);
    my $tmpoutfile = "$outfile-$hostname-$$";
    if ($this->{printcommand}) {
	open(OUT,">$tmpoutfile") or die "Can't open $tmpoutfile for write: $!";
	print OUT "command: $cmd\n" or die "print failed: $!";
	close(OUT) or die "close failed: $!";
	$cmd .= ">>$tmpoutfile 2>&1";
    } else {
	$cmd .= " >$tmpoutfile 2>&1";
    }
    print "Running '$cmd'\n";
    my $ret = system($cmd);
    if ($ret == 0) {
	rename($tmpoutfile,$outfile)
	    or die "Can't rename $tmpoutfile to $outfile: $!";
	exit(0);
    }
    warn "FAILED: $cmd\n";
    exit(1);
};

sub rebuild_thing_success {    
    my($this,$thing_info) = @_;

    return;
}

sub rebuild_thing_fail {
    my($this,$thing_info) = @_;

    return;
}
	    
1;

    
