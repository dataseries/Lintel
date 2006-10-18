#
#  (c) Copyright 2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

package BatchParallel::common;
use strict;
use warnings;

# the intent is that the major version will change on incompatible
# changes, and the minor version will change on forward-compatible
# changes.

$BatchParallel::common::interface_version = 1.0;
$BatchParallel::common::interface_version if 0; # eliminate warning

sub shuffle {
    my ($this,$arrayref) = @_;
    for(my $i=1;$i<@$arrayref;++$i) {
	my $rval = int(rand($i));
	($arrayref->[$i],$arrayref->[$rval]) = ($arrayref->[$rval],$arrayref->[$i]);
    }
}

use Cwd 'abs_path';
use File::Find;

sub new {
    my $class = shift;

    my $this = bless {}, $class;
    if (@_ != 0) {
	if ($_[0] eq 'help') {
	    $this->usage();
	    exit(0);
	}
	die "unknown options specified for batch-parallel module $class: '@_'";
    }
    return $this;
}

sub usage {
    die "you need to override sub usage in your module to give a usage message";
}

sub file_is_source {
    die "you need to override file_is_source or find_things_to_build in your module";
}

sub default_source_locations {
    die "you need to specify source locations or override default_source_locations in your module";
}

sub file_older { # true if $name_a is older than any of the later names
    my $this = shift;
    my $name_a = shift;

    my $time_a = -M $name_a;
    die "internal error $name_a doesn't exist"
	unless defined $time_a;
    foreach my $name_b (@_) {
	my $time_b = -M $name_b;
	die "unable to determine if $name_b is newer than $name_a as $name_b doesn't exist"
	    unless defined $time_b;
	return 1 if $time_a > $time_b;
    }
    return 0;
}

sub destfile_out_of_date {
    my($this,$prefix,$fullpath,$destfile) = @_;

    return $this->file_older($destfile,$fullpath);
}

sub my_abs_path {
    my ($thing) = @_;

    return abs_path($thing) if -d $thing;
    if ($thing =~ m!^(.+)/([^/]+)$!o) {
	my($p1,$p2) = ($1,$2);
	return abs_path($p1) . "/$p2";
    } else {
	return abs_path(".") . "/$thing";
    }
}

sub fullpath {
    my (@dirs) = @_;

    my @ret = map { my_abs_path($_) } @dirs;
    return @ret;
}

# routine can return any type of things_to_build array, but unless the
# rebuild_thing_{message,do,success,fail} routines are overriden, it
# must return an array of [ $prefix, $fullpath, $destfile ] elements

sub determine_things_to_build {
    my($this,$possible) = @_;

    my @things_to_build;
    foreach my $ent (@$possible) {
	my ($prefix,$fullpath) = @$ent;

	my $destfile = $this->destination_file($prefix,$fullpath);
	if (-f $destfile) {
	    if ($this->destfile_out_of_date($prefix,$fullpath,$destfile)) {
		push(@things_to_build, [ $prefix, $fullpath, $destfile ]);
	    }
	} else {
	    push(@things_to_build, [ $prefix, $fullpath, $destfile ]);
	} 
    }
    return @things_to_build;
}

sub find_things_to_build {
    my($this,@dirs) = @_;

    my $lastdot = time;
    my $source_count = 0;
    print "Finding files for batch-parallel operation:\n";
    
    @dirs = fullpath(@dirs);

    my @possibles;

    my $bp_select = sub {
	my($prefix) = @_;
    
	if ((time - $lastdot) >= 5) {
	    print ".";
	    $lastdot = time;
	}
	return unless $this->file_is_source($prefix,$File::Find::name,$_);
	++$source_count;
	push(@possibles,[$prefix,$File::Find::name]);
    };

    foreach my $prefix (@dirs) {
	print "  $prefix...";
	find(sub { &$bp_select($prefix) },$prefix);
	print "\n";
    }

    @possibles = sort { $a->[1] cmp $b->[1] } @possibles;

    my @things_to_build = $this->determine_things_to_build(\@possibles);
    return ($source_count, @things_to_build);
}

sub pre_exec_setup {
}

sub rebuild {
    die "You need to override sub rebuild in your module";
}

# thing_info in all of these functions is the bit that was put in each
# of the elements of the array returned by find_things_to_build

# attempt to verify that our find_things_to_build made the thing_info's
sub rebuild_sanity_check { 
    my($this,$thing_info) = @_;

    die "internal, mismatch of find_things_to_build and rebuild_thing_message"
        unless ref $thing_info eq 'ARRAY' && @$thing_info == 3;
    foreach my $subthing (@$thing_info) {
        die "internal, mismatch of find_things_to_build and rebuild_thing_message"
	    unless ref $subthing eq '';
    }
}

# this function generates a message if we are not actually executing
# the rebuild.

sub rebuild_thing_message {
    my($this,$thing_info) = @_;

    $this->rebuild_sanity_check($thing_info);
    my($prefix,$filepath,$destfile) = @$thing_info;

    print "should rebuild $filepath to $destfile\n";
}

# you will be forked before this is called; you should call exit to
# complete the function, it is an error to return.

sub rebuild_thing_do { 
    my($this,$thing_info) = @_;

    $this->rebuild_sanity_check($thing_info);
    my($prefix,$filepath,$destfile) = @$thing_info;

    exit(1) unless $this->rebuild($prefix,$filepath,"$destfile-new");
    exit(0);
};

sub rebuild_thing_success {    
    my($this,$thing_info) = @_;

    $this->rebuild_sanity_check($thing_info);
    my($prefix,$filepath,$destfile) = @$thing_info;

    if (-f "$destfile-new") {
	unlink($destfile);
	rename("$destfile-new",$destfile) || die "rename($destfile-new,$destfile) failed: $!";
    } else {
	die "child succeeded but $destfile-new doesn't exist?!\n";
    }
}

sub rebuild_thing_fail {
    my($this,$thing_info) = @_;

    $this->rebuild_sanity_check($thing_info);
    my($prefix,$filepath,$destfile) = @$thing_info;

    warn "child failed to successfully make $destfile-new from $filepath\n";
}

1;
