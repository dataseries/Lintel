#
#  (c) Copyright 2004-2007, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# TODO: merge this into Net::SSH::AuthorizedKeysFile

package Lintel::Net::SSH::KnownHostsFile;
use strict;
use warnings;
require Exporter;
use vars qw(@ISA @EXPORT_OK);

@ISA = qw(Exporter);
@EXPORT_OK = qw(has_host_key clear_host_key);

use Socket;
use Digest::HMAC;
use Digest::SHA1;
use MIME::Base64;
use FileHandle;

my $fh = new FileHandle "ssh -v 2>&1 |"
    or die "Can't run ssh -v: $!";
$_ = <$fh>;
die "No return from ssh -v?"
    unless defined $_;
die "Unexpected output from ssh -v, expected ^OpenSSH_\\d+\\.; got: $_ "
    unless $_ =~ /^OpenSSH_(\d+)\./o;
close($fh);
my $ssh_major_version = $1;
my $ssh_supports_hashed_keys = $ssh_major_version >= 4;

sub has_host_key {
    my($host) = @_;

    my @knownhosts = read_host_keys();
    my $start_count = @knownhosts;
    remove_host_key($host, \@knownhosts, 1);
    return $start_count != @knownhosts;
}

sub clear_host_key {
    my($host) = @_;

    my @knownhosts = read_host_keys();
    my $start_count = @knownhosts;
    remove_host_key($host, \@knownhosts);

    if (scalar @knownhosts != $start_count) {
	print "removing $host from $ENV{HOME}/.ssh/known_hosts...\n";
	open(HOSTS,">$ENV{HOME}/.ssh/known_hosts") or die "bad";
	print HOSTS @knownhosts;
	close(HOSTS);
    }
}

### Internal functions

sub read_host_keys {
    open(HOSTS,"$ENV{HOME}/.ssh/known_hosts") or die "bad";
    my @knownhosts = <HOSTS>;
    close(HOSTS);

    return @knownhosts;
}

sub remove_host_key {
    my($host, $knownhosts, $exact_match) = @_;

    my($name,$aliases,$addrtype,$length,@addrs) = gethostbyname($host);
    die "Host $host doesn't exist??" unless defined $name;
    my $ipaddr = inet_ntoa($addrs[0]);

    if ($exact_match) {
	@$knownhosts = grep(!/^$host[, ]/i, @$knownhosts);
    } else {
	@$knownhosts = grep(!/^$host[, ]/i,@$knownhosts);
	@$knownhosts = grep(!/^(\S+,)?$ipaddr /,@$knownhosts);
    }

    # Only look for hashed keys if the ssh implementation supports it,
    # otherwise we can give false positives for has_host_key

    return unless $ssh_supports_hashed_keys;
    for(my $i=0; $i < @$knownhosts; ++$i) {
	next unless $knownhosts->[$i] 
	    =~ m!^\|1\|([A-Za-z0-9\+/=]+)\|([A-Za-z0-9\+/=]+) !o;
	my($salt, $hash) = ($1,$2);
	if (hmac_equal($salt, $hash, $host) || 
	    hmac_equal($salt, $hash, $ipaddr)) {
	    splice(@$knownhosts, $i, 1);
	    --$i;
	}
    }
}

sub hmac_equal {
    my($b64salt, $hash, $val) = @_;

    my $salt = decode_base64($b64salt);
    my $hmac = Digest::HMAC->new($salt, "Digest::SHA1");
    $hmac->add($val);
    my $b64 = $hmac->b64digest();
    while (length $b64 < length $hash) {
	$b64 .= "="; # trailing "0"s left off by the hmac.
    }
    return $hash eq $b64;
}

1;

=pod

=head1 NAME

Lintel::Net::SSH::KnownHostsFile - manipulate the ssh .known_hosts file

=head1 DESCRIPTION

    use Lintel::Net::SSH::KnownHostsFile qw(has_host_key clear_host_key);
    print "has key for sunflower.hpl.hp.com"
        if has_host_key('sunflower.hpl.hp.com');
    clear_host_key('reinstalled.hpl.hp.com');

=head1 FUNCTIONS

=head2 has_host_key($hostname)

Returns true if the host key is in ~/.known_hosts, and false
otherwise.  Attempts to emulate exactly what ssh does such that if the
function returns false, ssh will ask you if you want to continue
connecting, and if it return true ssh will automatically connect.

=head2 clear_host_key($hostname)

Removes the host key for $hostname from the ~/.known_hosts file;
prints a message if it does so.

