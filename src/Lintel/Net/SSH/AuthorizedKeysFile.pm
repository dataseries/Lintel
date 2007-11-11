#
#  (c) Copyright 2004-2007, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# TODO: merge this into Net::SSH::AuthorizedKeysFile

package Lintel::Net::SSH::AuthorizedKeysFile;
use strict;
use warnings;
require Exporter;
use vars qw(@ISA @EXPORT_OK);

@ISA = qw(Exporter);
@EXPORT_OK = qw(clear_host_key);

use Socket;
use Digest::HMAC;
use Digest::SHA1;
use MIME::Base64;

sub clear_host_key {
    my($host) = @_;

    my($name,$aliases,$addrtype,$length,@addrs) = gethostbyname($host);
    die "Host $host doesn't exist??" unless defined $name;
    my $ipaddr = inet_ntoa($addrs[0]);
    open(HOSTS,"$ENV{HOME}/.ssh/known_hosts") or die "bad";
    my @knownhosts = <HOSTS>;
    close(HOSTS);
    my $start_count = scalar @knownhosts;
    @knownhosts = grep(!/^$host[, ]/i,@knownhosts);
    @knownhosts = grep(!/^(\S+,)?$ipaddr /,@knownhosts);
    for(my $i=0; $i < @knownhosts; ++$i) {
	next unless $knownhosts[$i] =~ m!^\|1\|([A-Za-z0-9\+/=]+)\|([A-Za-z0-9\+/=]+) !o;
	my($salt, $hash) = ($1,$2);
	if (hmac_equal($salt, $hash, $host) || 
	    hmac_equal($salt, $hash, $ipaddr)) {
	    splice(@knownhosts, $i, 1);
	    --$i;
	}
    }
    if (scalar @knownhosts != $start_count) {
	print "removing $host from $ENV{HOME}/.ssh/known_hosts...\n";
	open(HOSTS,">$ENV{HOME}/.ssh/known_hosts") or die "bad";
	print HOSTS @knownhosts;
	close(HOSTS);
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
