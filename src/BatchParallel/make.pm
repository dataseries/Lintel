package BatchParallel::make;
use Carp;
use strict;
use vars '@ISA';

die "module version mismatch" 
    unless $BatchParallel::common::interface_version < 2;

@ISA = qw/BatchParallel::common/;

sub usage {
    print <<END_OF_USAGE;
batch-parallel make transform=<perl-expr> command=<cmd> [command=<cmd> ...]
                    [printcmd] [debug-transform] -- file|dir ...

  This module will search for all files under the listed file or
  directories that are changed by the transform, i.e. the value of \$_
  is different after evaluating the expression.  If any of the output
  names already exist, then the input file will be ignored.  For each
  of those files it will then apply the commands listed (multiple ones
  can be specified).  Before running the command, it will substitute
  \$< with the input name, \$@ with the output name + -tmp, and \$TMP
  with a temporary directory for this job.  If all of the commands
  succeed (exit value 0), it will move the output file into the final
  location.  Otherwise it will leave the \$@-tmp file alone.
  Regardless, it will delete all the files under \$TMP.

END_OF_USAGE
}

sub new {
    my $class = shift;
    
    my $this = { 'commands' => [] };
    foreach my $arg (@_) {
	if ($arg eq 'help') {
	    usage();
	    exit(0);
	} elsif ($arg eq 'printcmd') {
	    $this->{printcommand} = 1;
	} elsif ($arg =~ /^transform=(.+)$/o) {
	    die "Can't specify two transforms" 
		if defined $this->{transform};
	    $this->{transform} = $1;
	} elsif ($arg =~ /^command=(.+)$/o) {
	    push(@{$this->{commands}}, $1);
	} elsif ($arg eq 'debug-transform') {
	    $this->{debug_transform} = 1;
	} else {
	    usage();
	    die "unable to interpret argument '$arg'."
	}
    }
    die "Did not specify a transform"
	unless defined $this->{transform};
    my $found_dollar_less = 0;
    my $found_dollar_at = 0;
    map { $found_dollar_less = 1 if /\$\</o; 
	  $found_dollar_at = 1 if /\$\@/o; } @{$this->{commands}};
    die "Did not specify a command that included \$<"
	unless $found_dollar_less;
    die "Did not specify a command that included \$@"
	unless $found_dollar_at;

    return bless $this, $class;
}

sub transform {
    my($this, $path) = @_;

    confess "??" unless defined $path;
    local $_ = $path;
    eval $this->{transform};
    die "Eval of '$this->{transform}' failed: $@"
	if $@;
    die "Eval of '$this->{transform}' returned an empty path"
	unless defined $_ && $_ ne '';
    print "DEBUG(via $this->{transform}): $path -> $_\n"
	if $this->{debug_transform};
    return $_;
}
    
sub file_is_source {
    my($this, $prefix, $src_path, $file_name) = @_;

    my $new_path = $this->transform($src_path);

    return $src_path ne $new_path;
}

sub destination_file {
    my($this, $prefix, $src_path) = @_;

    return $this->transform($src_path);
}

# returns 0 on failure, 1 on success
sub do_rebuild {
    my($this, $src_path, $dest_path, $tmpdir, $mock) = @_;

    die "??" unless $tmpdir =~ m!^/!o;
    foreach my $cmd (@{$this->{commands}}) {
	$cmd =~ s/\$</$src_path/g;
	$cmd =~ s/\$@/$dest_path/g;
	$cmd =~ s/\$TMP/$tmpdir/g;

	print "  " if $mock;
	print "$cmd\n" if $this->{printcommand} || $mock;
	unless ($mock) {
	    my $ret = system($cmd);
	    if ($ret) {
		print "$cmd failed, returned $ret\n";
		return 0;
	    }
	}
    }
    return 1;
}

sub rebuild {
    my($this, $prefix, $src_path, $dest_path) = @_;

    my $tmpdir = "/tmp/bp.make.tmp.$$";
    mkdir($tmpdir, 0777) or die "Can't mkdir $tmpdir: $!";

    my $ok = $this->do_rebuild($src_path, $dest_path, $tmpdir, 0);

    my $cleanup_ok = system("rm -rf $tmpdir");
    die "Unable to cleanup $tmpdir" unless $cleanup_ok == 0;
    
    return $ok;
}

sub rebuild_thing_message {
    my($this, $thing_info) = @_;

    $this->rebuild_sanity_check($thing_info);
    my ($prefix, $srcpath, $destpath) = @$thing_info;
    print "$srcpath -> $destpath:\n";
    $this->do_rebuild($srcpath, "$destpath-new", '/tmp/tmp-work-dir', 1);
    print "  mv $destpath-new $destpath\n";
    print "\n";
}

1;

