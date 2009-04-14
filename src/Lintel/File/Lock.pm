package Lintel::File::Lock;
use strict;
use warnings;
use Fcntl qw/:flock :DEFAULT/;
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

=head2 getLock(filename, [waittime, verbose, flags, mode])

   my $fh = getLock($filename, $waittime, $verbose, $flags, $mode);
   my $fh = getLockEx($filename,
                      waittime => $waittime,
		      verbose => $verbose,
		      flags => $flags,
		      mode => $mode);

getLock will attempt to acquire a lock on filename.  By default, the 
file will be created if it doesn't exist, and getLock will return
an exclusive locked read/write FileHandle on success.  If you are going to
write to the file, you should call one of C<seek($fh, 0, SEEK_END); or
seek($fh, 0, SEEK_SET);> to set the current writing position.

To release the lock, set the filehandle to undef.  

If no waittime is specified, it will wait forever.  Otherwise if it 
has waited for waittime seconds, it will return undef.  Various additional 
information is printed to stderr if verbose is specified.  

The flags parameter is a bitwise or of the flags to sysopen.  The
default is to use O_CREAT | O_RDWR.

The mode parameter is used to change the access rights to the file for
a file that is being newly created.  If the mode parameter is omitted
it defaults to 0666.

If some rare errors occur getLock will die.  Locking requests may be silently
ignored by some filesystems, and in particular has known errors in many NFS
implementations.  Recent Linux NFS clients talking to a recent Linux NFS server
have functional locking.

=cut

sub getLockEx {
    my ($filename, %param) = @_;
    return getLock($filename, $param{waittime}, $param{verbose}, 
	            $param{flags}, $param{mode});
}

sub getLock { 
    my ($filename, $waittime, $verbose, $flags, $mode) = @_;
    die "?" unless defined $filename;
    $flags ||= O_CREAT | O_RDWR;
    $mode ||= 0666;

    my $fh;
    sysopen($fh, $filename, $flags, $mode) 
	or die "Unable to create or open $filename for read/write: $!";
    if (defined $waittime) {
	die "??" if $waittime < 0;
	my $start = time;
	while(1) {
	    my $ret = flock($fh, LOCK_EX|LOCK_NB);
	    if ($ret) {
		return $fh;
	    } elsif (! defined $ret) {
	        die "flock($filename, LOCK_EX|LOCK_NB) failed: $!";
	    }

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
    die "internal error";
}

=pod

=head1 AUTHOR

Eric Anderson <anderse@hpl.hp.com>

=head1 SEE ALSO

lintel-flock(1), perl(1)

=cut

1;

