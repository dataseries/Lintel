package Plot::Mercury;

#
#  (c) Copyright 2004-2006, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Module for use by mercury-plot; provides various functions for use
# in the Safe evaluation env.

use strict;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK $Default_DSN $dbh);

use DBI;
use English;
use Carp;

require Exporter;
require AutoLoader;

@ISA = qw(Exporter AutoLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
	     sql_quote sql_prepare sql_command sql_unique_row sql_exec reload_use
);
$VERSION = '0.02';

$Default_DSN = $ENV{MERCURY_DSN};
if (!defined $Default_DSN && -f "$ENV{HOME}/.mercury-dbs") {
    ($Default_DSN,$_,$_) = getDSNbykey("default", "$ENV{HOME}/.mercury-dbs");
}   

if (!defined $Default_DSN && -f "/etc/MercuryDBs") {
    ($Default_DSN,$_,$_) = getDSNbykey("default", "/etc/MercuryDBs");
}

unless (defined $Default_DSN) {
    $Default_DSN = "DBI:mysql:database=test;host=localhost";
}

sub getDSNbykey {
    my($dbkey,$file) = @_;

    return () unless -r $file;
    my $fh = new FileHandle $file
	or die "Unable to open $file for read: $!";
    while(<$fh>) {
	chomp;
	if (/^$dbkey\s+(.+)$/) {
	    my $bits = $1;
	    $bits =~ s/\s+$//o;
	    my @bits = split(/\s+/,$bits);
	    die "Invalid line in $file: '$_'"
		if @bits < 1 || @bits > 2;
	    die "Invalid DSN '$bits[0]' in $file"
		unless $bits[0] =~ /^DBI:/o;
	    return @bits;
	}
    }
    return ();
}

my @connect_parms;

sub reconnectToDB {
    die "Can't reconnectToDB without having first connected!\n"
	unless defined $dbh;
    my $ret;

    ($ret) = eval { sql_unique_row("select 1 from experiments limit 1"); };
    return 1 if $@ eq '' && $ret == 1;

    while(1) {
	if ($@ =~ /Lost connection/o) {
	    $|=1;
	    print "Reconnecting...";
	    if (defined $dbh) {
		$dbh->disconnect();
		$dbh = undef;
	    }
	    $ret = eval { $_[0]->connectToDB(@connect_parms); };
	    return 1 if defined $dbh && $ret == 1;
	    if ($@ =~ /Unable to connect to/o) {
		print "reconnect failed, sleep(15), retry\n";
		sleep(15);
		$@ = 'Lost connection';
		next;
	    }
	    die "Reconnect failed: $@\n";
	} else {
	    die "Unable to reconnect: $@\n";
	}
    }
}

sub connectToDB {
    my ($class,$dsn,$user,$password) = @_;

    die "Already connected to database!\n"
	if defined $dbh;
    $dsn = $Default_DSN unless defined $dsn;
    @connect_parms = ($dsn,$user,$password);
    die "Can't connectToDB without a dsn. set \$Mercury_DSN in your environment, 
  create /etc/MercuryDSN, or run the program with a --dsn=... argument.\n"
      unless defined $dsn;

    $Plot::Mercury::dbh = eval { DBI->connect(@connect_parms); };
    die "Unable to connect to $dsn: $@\n"
	unless defined $dbh;
    $dbh->{Warn} = 1;
    $dbh->{RaiseError} = 1;
    return 1;
}

sub add_col {
    my($colsdef,$k,$v) = @_;

    die "Invalid column name $k, can't start with infra_\n"
	if $k =~ /^infra_/o;
    die "Invalid column name $k, should match /^\\w+$/\n"
	unless $k =~ /^\w+$/o;
    
    push(@$colsdef,"$k $v");
}

sub sql_quote {
    my($in) = @_;

  Plot::Mercury->connectToDB() unless defined $dbh;
    return $dbh->quote($in);
}

sub sql_prepare {
    my($sql) = @_;

  Plot::Mercury->connectToDB() unless defined $dbh;
    my $sth = $dbh->prepare($sql);
    die "prepare of '$sql' failed!\n" unless defined $sth;
    return $sth;
}

sub sql_exec {
    my ($sql) = @_;

    my $sth = sql_prepare($sql);
    my $rc;
    eval {
	$rc = $sth->execute();
    };
    confess ("execution of '$sql' failed: " . $dbh->errstr)
	unless $@ eq '' && $rc;
    return $sth;
}

sub sql_unique_row {
    my ($sql) = @_;
    
    my $sth = sql_exec($sql);
    die "'$sql' did not give a single unique row!"
	unless $sth->rows == 1;
    my @data = $sth->fetchrow_array;
    return @data;
}

sub sql_command {
    my ($sql,$expect_rows) = @_;

    $expect_rows ||= 0;
    my $sth = sql_exec($sql);
    # Wanted to check that the command didn't return any rows, but
    # insert commands return a row count, and then complain on fetchrow_array.
    return if defined $expect_rows && $expect_rows eq '*';
    my $rows = $sth->rows;
    die "SQL command $sql failed.  Expected $expect_rows, got $rows\n"
	unless $expect_rows == $rows;
}

sub create_experiment {
    my ($class,$expinfo,@extra_cols) = @_;

    die "To create a new experiment, an experiment name must be specified!\n"
	unless defined $expinfo->{name};
    die "Experiment ID should not be defined when creating an experiment!\n"
	if defined $expinfo->{expid};

    $expinfo->{author} = getpwuid($UID)
	unless defined $expinfo->{author};
    ($expinfo->{created}) = sql_unique_row("select NOW()")
	unless defined $expinfo->{created};
    my $name = sql_quote($expinfo->{name});
    my $author = sql_quote($expinfo->{author});
    my $created = $expinfo->{created};
    $created = sql_quote($created);
    my $description = defined $expinfo->{description} ? 
	sql_quote($expinfo->{description}) : "''";

    my @data_cols;
    while (my($k,$v) = each %{$expinfo->{inputs}}) {
	die "Can't have input column with same name ($k) as output column\n"
	    if defined $expinfo->{outputs}->{$k};
	add_col(\@data_cols,$k,$v);
    }
    die "When creating a table, must specify at least one input parameter\n"
	unless @data_cols > 0;

    while(my($k,$v) = each %{$expinfo->{outputs}}) {
	add_col(\@data_cols,$k,$v);
    }

    my @t = sql_command("insert into experiments values (NULL, $name, $author, $created, $description)",1);

    my ($expid) = sql_unique_row("select expid from experiments where name = $name and author = $author and created = $created");
    
    sql_command("create table exptrials_$expid ( trialid int not null auto_increment, key(trialid), status set('pending','running','succeeded','failed','suspended') not null, exec_host varchar(64), exec_pid int, exec_begin datetime, exec_end datetime, " . join(", ",@extra_cols, @data_cols) . ")");
    
    return $expid;
}

sub find_experiment {
    my($class,$exp_info) = @_;

    my @restrict;

    push(@restrict, "expid = $exp_info->{expid}")
	if defined $exp_info->{expid};
    push(@restrict, "name = " . sql_quote($exp_info->{name}))
	if defined $exp_info->{name};
    push(@restrict, "author = " . sql_quote($exp_info->{author}))
	if defined $exp_info->{author};
    push(@restrict, "created = " . sql_quote($exp_info->{created}))
	if defined $exp_info->{created};

    die "To find an experiment you must specify at least one of expid, name, author, or created.\n"
	unless @restrict > 0;
    my $r = join(" and ",@restrict);
    my $sth = sql_exec("select expid, name, author, created from experiments where $r");
    if ($sth->rows() == 0) {
	die "restriction set '$r' did not find any experiment??\n";
    } elsif ($sth->rows() == 1) {
	my @data = $sth->fetchrow_array;
	$exp_info->{expid} = $data[0];
	$exp_info->{name} = $data[1];
	$exp_info->{author} = $data[2];
	$exp_info->{created} = $data[3];
	return $exp_info->{expid};
    } else {
	print STDERR "restriction set '$r' found multiple experiments??\n";
	print STDERR "expid, name, author, created:\n";
	while (my @row = $sth->fetchrow_array) {
	    print STDERR join(", ",@row);
	}
	die "restriction set must select unique experiment!";
    }
}
	    
# reload_use will force a reload of a changed module -- 
# this is useful in mercury scripts if we need to improve
# a module that is being used to avoid having to restart 
# mercury from scratch.

my %module_times;

sub reload_use {
    my($module_orig,$verbose) = @_;

    my $module = $module_orig;
    $module =~ s!::!/!go;
    foreach my $inc (@INC) {
	my $file = "$inc/${module}.pm";
	if (-f $file) {
	    if (defined $module_times{$file} && -M $file == $module_times{$file}) {
		print "$module has not changed\n"
		    if $verbose;
		return; 
	    }
	    my $ret = do $file;
	    die "unable to read $file: $!" if $!;
	    die "unable to parse $file: $@" if $@;
	    die "$file didn't return true" unless $ret;

	    $module_times{$file} = -M $file;
	    print "reloaded $module\n"
		if $verbose;
	    return;
	}
    }
    die "unable to find module $module_orig in @INC";
}

1;
__END__
# Below is the stub of documentation for your module. You better edit it!

=head1 NAME

Plot::Mercury - Perl extension for providing features to mercury-plot scripts

=head1 SYNOPSIS

see C<mercury-plot --man>; this module is not intended to be directly used.

=head1 DESCRIPTION

A module for internal use by mercury-plot, not for anything else

=head1 AUTHOR

Eric Anderson <anderse@hpl.hp.com>

=head1 SEE ALSO

mercury-plot(1), perl(1).

=cut
