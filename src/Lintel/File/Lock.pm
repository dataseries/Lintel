package Lintel::File::Lock;
use strict;
use warnings;
use Fcntl ':flock';
use FileHandle;

=pod

=head1 NAME

Lintel::File::Lock - locking and unlocking of files

=head1 SYNOPSIS

  use Lintel::File::Lock;

  # Wait forever
  my $fh = Lintel::File::Lock::getLock("/tmp/lock");
  die "??" unless defined $fh

  # Wait for 10 seconds
  my $fh = Lintel::File::Lock::getLock("/tmp/lock", 10);
  warn "no lock" unless defined $fh;

  # release lock
  $fh = undef;

=head1 FUNCTIONS

=head2 getLock(filename, waittime, verbose)

getLock will attempt to acquire a lock on filename.  It will return a
locked rw FileHandle on success.  To release the lock, simply set the
filehandle to undef.  If no waittime is specified, it will wait
forever.  Otherwise if it has waited for waittime seconds, it will
return undef.  Various additional information is printed to stderr if
verbose is specified.  If some rare errors occur it will die.

=cut

sub getLock { 
    my ($filename, $waittime, $verbose) = @_;
    die "?" unless defined $filename;

    my $fh = new FileHandle "+>>$filename"
	or die "Unable to open $filename for rw append: $!";
    if (defined $waittime) {
	die "??" if $waittime < 0;
	my $start = time;
	while(1) {
	    my $ret = flock($fh, LOCK_EX|LOCK_NB);
	    unless(defined $ret) {
	        die("flock($filename, LOCK_EX|LOCK_NB) failed: $!");
	    }
	    return $fh if $ret;

	    my $remain = $waittime - (time - $start);
	    print STDERR "delayed waiting for lock, $remain seconds remain...\n"
		if $verbose;
	    return undef
		if (time - $start) >= $waittime;
	    sleep(1);
	}
    } else {
	unless(flock($fh,LOCK_EX)) {
	    die "flock($filename, LOCK_EX) failed: $!";
	}
	return $fh;
    }
    return $fh;
}

=pod

=head1 AUTHOR

Eric Anderson <anderse@hpl.hp.com>

=head1 SEE ALSO

lintel-flock(1), perl(1)

=cut

1;

