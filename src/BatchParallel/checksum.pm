#
#  (c) Copyright 2004-2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

package BatchParallel::checksum;
use strict;
use warnings;
use vars '@ISA';
use FileHandle;
use Digest::MD5;
use Lintel::SHA1;

die "module version mismatch"
    unless $BatchParallel::common::interface_version < 2;

@ISA = qw/BatchParallel::common/;

sub usage {
    print <<END_OF_USAGE;
batch-parallel checksum [sha1 (default)] [md5] [uncompressed-also]
  [basename] [match=<regex>]
  If uncompressed-also is specified, checksums for the uncompressed
  contents of gzip or bzip2 compressed files will also be calculated.
  Output is <file>.checksum; files matching *.checksum or *.checksum-new
  will not be checksummed.  Multiple checksums can be calculated in 
  one pass.  [basename] means that the filenames written to the checksum
  file should not include the directory.  match specifies a regex to require
  for the filenames to be checksummed.
END_OF_USAGE
}

sub new {
    my $class = shift;
    
    my $this = { 'digests' => {},
		 'uncompressed_also' => 0,
		 'basename' => 0 };
    foreach my $arg (@_) {
	if ($arg eq 'sha1' || $arg eq 'md5') {
	    $this->{digests}->{$arg} = 1;
	} elsif ($arg eq 'uncompressed-also') {
	    $this->{uncompressed_also} = 1;
	} elsif ($arg eq 'basename') {
	    $this->{basename} = 1;
	} elsif ($arg =~ /^match=(.+)$/o) {
	    my $re = $1;
	    $this->{match} = qr/$re/;
	} elsif ($arg eq 'help') {
	    usage();
	    exit(0);
	} else {
	    usage();
	    die "unable to interpret argument '$arg'."
	}
    }
    $this->{digests}->{sha1} = 1
	unless scalar keys %{$this->{digests}} > 0;
    return bless $this, $class;
}

sub file_is_source {
    my($this,$prefix,$fullpath,$filename) = @_;

    return 0 unless -f $filename;
    return 0 if $filename =~ /\.checksum(-new)?$/o;
    return 0 if defined $this->{match} && $fullpath !~ $this->{match};
    return 1;
}

sub destination_file {
    my($this,$prefix,$fullpath) = @_;

    return "${fullpath}.checksum";
}

sub destfile_out_of_date {
    my($this,$prefix,$fullpath,$destfile) = @_;

    return 0 if -f $destfile;
    return 1;
}

sub fail {
    my $destpath = shift;

    unlink $destpath;
    die @_;
}

sub checksum {
    my($this, $source, $output, $filename) = @_;

    foreach my $digest_type (sort keys %{$this->{digests}}) {
        my $digest_class;
        $digest_class = $Lintel::SHA1::impl_package if $digest_type eq 'sha1';
        $digest_class = 'Digest::MD5' if $digest_type eq 'md5';
        die "unknown digest type '$digest_type'" unless defined $digest_class;

	my $digest = $digest_class->new();

	my $fh = new FileHandle($source)
	    or die "Can't open $source for read: $!";
	$digest->addfile($fh);
	my $checksum = $digest->hexdigest();

	print $output "$digest_type $checksum $filename\n";
    }
}

sub rebuild {
    my($this,$prefix,$fullpath,$destpath) = @_;

    my $output = new FileHandle ">$destpath"
	or die "Can't open $destpath for write: $!";

    print "Calculating checksums for $fullpath\n";
    my $basename = $fullpath;
    $basename =~ s!^.*/!!o if $this->{basename};
    $this->checksum($fullpath, $output, $basename);
    if ($this->{uncompressed_also} && $fullpath =~ /\.((bz2)|(gz))$/o) {
	$basename =~ s/\.((bz2)|(gz))$//o
	    or die "??";
	my $uncompress;
	$uncompress = "gunzip -c" if $fullpath =~ /\.gz$/o;
	$uncompress = "bunzip2 -c " if $fullpath =~ /\.bz2$/o;
	$this->checksum("$uncompress < $fullpath |", $output, $basename);
    }

    close($output);
    return 1;
}

1;
