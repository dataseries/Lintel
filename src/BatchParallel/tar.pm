#
#  (c) Copyright 2006-2007, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

package BatchParallel::tar;
use strict;
use warnings;
use FileHandle;
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
	} elsif (/^verbose=(\d+)$/o) {
	    $this->{verbose_level} = $1;
	    print "$this->{verbose_level} XX\n";
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
    [sort={none,lexical}] [path-parts[=#]] [verbose=#]
  -- file/directory...

  The tar module will find all of the files under the files and
  directories specified, sort them (if requested), and create a series
  of tar files under the destination directory (default /tmp).  Each
  tar file will be a contiguous set of files such that the estimated
  size of the tar file is less than the target size.  Each output file
  will be named by the first and last file in it's group, the path
  from each file will retain all components unless rpath-parts is specified, 
  in which case only the number of components specified (default 1)
  are retained, and then the /'s in the path will be replaced with _.
  The same rule for stripping off path components will be applied to files
  stored in the tar archives, except for the / to _ transform.

  Default verbose level is 1, level 2 prints out the temporary files that
  will be used by tar to make the tarfiles if run in -n mode.
END_OF_USAGE
}

sub pathToParts ($$) {
    my($this, $path) = @_;

    my $spath = $path;
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
    my $upath = $path;
    $upath =~ s!/!_!go;
    print "$spath -> $path\n" if 0;
    return join('/', @first), $path, $upath;
}

sub newThing {
    my($this, $cur_group) = @_;

    my ($firstdir, $firstpath, $firstname) 
	= $this->pathToParts($cur_group->[0]);
    my ($lastdir, $firstpath, $lastname) 
	= $this->pathToParts($cur_group->[@$cur_group - 1]);

    my $dest_file = "$this->{destdir}/$firstname";
    if ($firstname ne $lastname) {
	$dest_file .= "--$lastname";
    }
    $dest_file .= ".tar";
    die "Duplicate destination $dest_file, do you need to increase the number of components retained by path-parts?"
	if defined $this->{dest_files}->{$dest_file};
    $this->{dest_files}->{$dest_file} = 1;
    
    my @group;
    foreach my $ent (@$cur_group) {
	my($dir, $path, $name) = $this->pathToParts($ent);
	push(@group, "-C$dir\n")
	    unless $dir eq '';
	push(@group, "$path\n");
    }

    return BatchParallel::Tar::Thing->new(\@group, $dest_file);
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
    push(@groups, $this->newThing($cur_group));
    my $source_count = @groups;

    delete $this->{dest_files};
    return ($source_count, @groups);
}

sub file_is_source {
    my($this,$prefix,$fullpath,$filename) = @_;

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

    $thing->rebuild_thing_success();
}

sub rebuild_thing_fail {
    my($this,$thing) = @_;

    $thing->rebuild_thing_fail();
}

sub rebuild_thing_message {
    my($this,$thing) = @_;

    $thing->rebuild_thing_message($this);
}
    
package BatchParallel::Tar::Thing;

sub new {
    my($class, $srcs, $dest) = @_;

    return bless { 'srcs' => $srcs,
		   'dest' => $dest };
}

sub rebuild_thing_do {
    my($thing, $this) = @_;

    my $filelist = "/tmp/batch-parallel.tar.$$";
    open(FILELIST, ">$filelist")
	or die "Unable to write filelist $filelist: $!";
    my $data = join('', @{$thing->{srcs}});
    print FILELIST $data;
    close(FILELIST) or die "close failed: $!";
    die "Something wrong with write to $filelist; size mismatch"
	unless -s $filelist == length $data;
    my $cmd = "tar -c -f $thing->{dest} -S --files-from $filelist";
    
    $this->run($cmd);
    unlink($filelist);
    exit(0);
}

sub rebuild_thing_success {
    my($this) = @_;

}

sub rebuild_thing_fail {
    my($this) = @_;

}

sub rebuild_thing_message {
    my($thing, $this) = @_;

    print "Should build $thing->{dest}\n";
    if ($this->{verbose_level} >= 2) {
	print map { "  $_" } @{$thing->{srcs}};
	print "\n";
    }
}

1;

