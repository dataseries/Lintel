#
#  (c) Copyright 2006-2007, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

package BatchParallel::tar;
use strict;
use warnings;
use FileHandle;
use Carp;
use vars '@ISA';
@ISA = qw/BatchParallel::common/;

die "BatchParallel::common version change?" 
    unless $BatchParallel::common::interface_version >= 1.0 
        && $BatchParallel::common::interface_version < 2;

$| = 1;

# FUTURE: add option to specify number of files and module picks the sizes
# FUTURE: add option to try to do tighter bin-packing of the files?

sub new {
    my $class = shift;

    my $this = bless {
	'target_size' => undef,
	'destdir' => undef,
	'sort' => 'none',
	'strip_path' => undef,
	'verbose_level' => 1
    }, $class;
    while (@_ > 0) {
	$_ = shift @_;
	if ($_ eq 'help') {
	    $this->usage();
	    exit(0);
	} elsif (/^target-size=(\d+(\.\d+)?)([kmg])?$/o) {
	    die "Can't set multiple target sizes" 
		if defined $this->{target_size};
	    my ($size,$units) = ($1,$3);
	    $units ||= '';
	    $size *= 1024 if $units eq 'k';
	    $size *= 1024 * 1024 if $units eq 'm';
	    $size *= 1024 * 1024 * 1024 if $units eq 'g';
	    $size = sprintf("%d", $size);
	    $this->{target_size} = $size;
	} elsif (/^destdir=(\S+)$/o) {
	    die "Can't set multiple destination directories"
		if defined $this->{destdir};
	    $this->{destdir} = $1;
	} elsif (/^path-parts(?:=(\d+))?$/o) {
	    $this->{path_parts} = $1 || 1;
	} elsif (/^strip-path=(\d+)$/o) {
	    $this->{strip_path} = $1;
	} elsif (/^require-re=(.+)$/o) {
	    $this->{require_re} = qr/$1/;
	} elsif (/^exclude-re=(.+)$/o) {
	    $this->{exclude_re} = qr/$1/;
	} elsif (/^verbose=(\d+)$/o) {
	    $this->{verbose_level} = $1;
	} else {
	    die "unknown options specified for batch-parallel module $class: '$_'";
	}
    }

    die "Specifying both strip-path and path-parts doesn't make sense"
	if defined $this->{strip_path} && defined $this->{path_parts};
    die "Must specify a target size > 0"
	unless defined $this->{target_size} && $this->{target_size} > 0;

    unless (defined $this->{destdir}) {
	$this->{destdir} = "/tmp";
	print "Using default destination directory /tmp\n";
    }

    return $this;
}

# TODO: move this into Pod::Usage or something
sub usage {
    print <<'END_OF_USAGE';
batch-parallel -j 1 --noshuffle tar target-size=#[kmg] [destdir=path]
    [require-re=regex] [exclude-re=regex] [sort={none,lexical}] 
    [path-parts[=#]] [strip-path=#] [verbose=#]
  -- file/directory...

  The tar module will find all of the files under the files and
  directories specified.  It will prune out any files matching
  exclude-re, and will require all the files to match require-re.
  Then it will take the list of files, sort them if requested, and
  create a series of tar files under the destination directory
  (default /tmp).  Each tar file will be a contiguous set of files
  such that the estimated size of the tar file is less than the target
  size.

  Each output tar file will be named by the first and last file in
  it's group.  The following transforms will be applied to the
  filenames: First, if path-parts was specified, only that many of the
  rightmost components will be retained; this is useful for irregular
  directory structures where some number of components on the right
  are sufficient for unique identification.  Second, if strip-path was
  specified, that many components will be removed from the leftmost
  part of the path; this is useful for removing some prefix of all the
  filenames that is unimportant.  Next the the /'s in the path will be
  replaced with _.  The same rule for removing path components will be
  applied to files stored in the tar archives, except for the / to _
  transform.

  Default verbose level is 1, level 2 prints out the temporary files that
  will be used by tar to make the tarfiles if run in -n mode.
END_OF_USAGE
}

sub pathToParts ($$) {
    my($this, $path) = @_;

    confess "internal: path to parts called without path??"
	unless defined $path;
    my $start_path = $path;
    my @first;
    if (defined $this->{path_parts}) {
	die "??" if $this->{path_parts} <= 0;
	my @parts = split(m!/!o, $path);
	while (@parts > $this->{path_parts}) {
	    push(@first, shift @parts);
	}
	$path = join("/", @parts);
    }
    if (defined $this->{strip_path}) {
	my @parts = split(m!/!o, $path);
	@first = splice(@parts, 0, $this->{strip_path});
	$path = join("/", @parts);
    }
    die "No path remains from $start_path?"
	unless defined $path;
    my $upath = $path;
    $upath =~ s!/!_!go;
    print "$start_path -> $path\n" if 0;
    return join('/', @first), $path, $upath;
}

sub newThing {
    my($this, $cur_group) = @_;

    confess "Internal, empty group?"
	unless @$cur_group > 0;
    map { die "Internal, empty path in group" 
	      unless defined $_ } @$cur_group;
    my ($firstdir, $firstpath, $firstname) 
	= $this->pathToParts($cur_group->[0]);
    my ($lastdir, $lastpath, $lastname) 
	= $this->pathToParts($cur_group->[@$cur_group - 1]);

    my $dest_file = "$this->{destdir}/$firstname";
    if ($firstname ne $lastname) {
	$dest_file .= "--$lastname";
    }
    $dest_file .= ".tar";
    die "Duplicate destination $dest_file, do you need to increase the number of components retained by path-parts?"
	if defined $this->{dest_files}->{$dest_file};
    $this->{dest_files}->{$dest_file} = 1;
    
    my @tar_file_list;
    my @srcs;
    foreach my $ent (@$cur_group) {
	push(@srcs, $ent);
	my($dir, $path, $name) = $this->pathToParts($ent);
	push(@tar_file_list, "-C$dir\n")
	    unless $dir eq '';
	push(@tar_file_list, "$path\n");
    }

    my $tar_file_list = join('', @tar_file_list);
    return BatchParallel::Tar::Thing->new($tar_file_list, \@srcs, $dest_file);
}

sub find_things_to_build {
    my($this, @dirs) = @_;

    my @possibles = $this->find_possibles(@dirs);

    if ($this->{sort} eq 'lexical') {
	@possibles = sort { $a->[1] <=> $b->[1] } @possibles;
    }

    my @groups;

    my $tar_file_overhead = 200; # guess.
    my $cur_groupsize = 0;
    my $cur_group = [];
    foreach my $ent (@possibles) {
	die "Can't handle filenames with newlines ($ent->[1])"
	    if $ent->[1] =~ /\n/o;
	die "Can't handle filenames starting with - ($ent->[1])"
	    if $ent->[1] =~ /^-/o; # tar limitation
	my $size = -s $ent->[1];
	die "$ent->[1] vanished?" unless defined $size;

	$size += $tar_file_overhead;
	die "individual file $ent->[1] + tar overhead ($size bytes) is larger than target size ($this->{target_size} bytes)"
	    if $size > $this->{target_size};
	if ($cur_groupsize + $size > $this->{target_size}) {
	    push(@groups, $this->newThing($cur_group));
	    $cur_group = [];
	    $cur_groupsize = 0;
	}
	$cur_groupsize += $size;
	push(@$cur_group, $ent->[1]);
    }
    push(@groups, $this->newThing($cur_group))
	if @$cur_group > 0;
    my $source_count = @groups;

    delete $this->{dest_files};
    @groups = grep($_->needs_rebuild($this), @groups);

    return ($source_count, @groups);
}

sub file_is_source {
    my($this,$prefix,$fullpath,$filename) = @_;

    return if defined $this->{exclude_re} && $fullpath =~ $this->{exclude_re};
    return if defined $this->{require_re} && $fullpath !~ $this->{require_re};
    warn "Found tar file $fullpath"
	if $fullpath =~ /\.tar\b/o && -f $fullpath;
    return 1 if -f $fullpath;
    return 0;
}

sub rebuild_thing_do {
    my($this,$thing) = @_;

    $thing->rebuild_thing_do($this);
}

sub rebuild_thing_success {
    my($this,$thing) = @_;

    $thing->rebuild_thing_success($this);
}

sub rebuild_thing_fail {
    my($this,$thing) = @_;

    $thing->rebuild_thing_fail($this);
}

sub rebuild_thing_message {
    my($this,$thing) = @_;

    $thing->rebuild_thing_message($this);
}
    
package BatchParallel::Tar::Thing;

sub new {
    my($class, $tar_file_list, $srcs, $dest) = @_;

    return bless { 'tar_file_list' => $tar_file_list,
		   'srcs' => $srcs,
		   'dest' => $dest 
		   };
}

sub rebuild_thing_do {
    my($thing, $this) = @_;

    my $filelist = "/tmp/batch-parallel.tar.$$";
    open(FILELIST, ">$filelist")
	or die "Unable to write filelist $filelist: $!";
    print FILELIST $thing->{tar_file_list};
    close(FILELIST) or die "close failed: $!";
    die "Something wrong with write to $filelist; size mismatch"
	unless -s $filelist == length $thing->{tar_file_list};
    my $cmd = "tar -c -f $thing->{dest}-tmp -S --files-from $filelist";
    
    $this->run($cmd);
    unlink($filelist);
    exit(0);
}

sub rebuild_thing_success {
    my($thing, $this) = @_;

    rename("$thing->{dest}-tmp", "$thing->{dest}")
	or die "rename from $thing->{dest}-tmp to $thing->{dest} failed: $!";

}

sub rebuild_thing_fail {
    my($thing, $this) = @_;

    warn "Rebuild of $thing->{dest} failed.";
}

sub rebuild_thing_message {
    my($thing, $this) = @_;

    print "Should build $thing->{dest}\n";
    if ($this->{verbose_level} >= 2) {
	my @tmp = split(/\n/o, $thing->{tar_file_list});
	grep(chomp, @tmp);
	print map { "  $_\n" } @tmp;
	print "\n";
    }
}

sub needs_rebuild {
    my($thing, $this) = @_;

    return $this->file_older($thing->{dest}, @{$thing->{srcs}});
}
1;

