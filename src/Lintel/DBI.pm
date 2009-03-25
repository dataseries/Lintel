package Lintel::DBI;
use English;
use DBI;
use vars '$AUTOLOAD';
use Carp;
use FileHandle;
use Data::Dumper;
use strict;

my %invalid_sth_names = map { $_ => 1 }
    qw/AUTOLOAD DESTROY fetchArray fetchHash oneRow atMostOneRow/;

=pod

=head1 NAME

Lintel::DBI - Front end to DBI that makes common cases easier

=head1 SYNOPSIS

    use Lintel::DBI;

    my $dbh = new Lintel::DBI();

    my $dbh = Lintel::DBI->connect();
    my $dbh = Lintel::DBI->connect('DBI:mysql:test:localhost',$user,$passwd);

    $dbh->load_sths('example1' => 'select sum(val) as sum_val from example where akey = ?',
		    'example2' => 'select distinct state from states where country = ?',
                    'example3' => 'select min(state) from states where country = ?',
		    'example4' => 'select * from states where state = ?');

    my $sum = $dbh->{sth}->example1('test')->oneRow()->{sum_val};
    my $sum = $dbh->{sth}->oneRow('example1','test')->{sum_val};
    my $sum = $dbh->{sth}->selectScalar('example1','test');

    my $sth = $dbh->{sth}->example2('U.S.');
    while(my ($state) = $sth->fetchArray()) {
	print $state, "\n";
    }
    while(my $row = $sth->fetchHash()) {
	print $row->{state}, "\n";
    }

    my ($state) = $dbh->{sth}->example3('no-country')->atMostOneRowArray();
    my ($state) = $dbh->{sth}->example3('Paraguay')->oneRowArray();
    my $info = $dbh->{sth}->example4('North Dakota')->oneRow();
    print "state = $info->{state}, population = $info->{population}\n";
    my @info = $dbh->{sth}->example4('South Dakota')->oneRowArray();
    print join(", ", @info), "\n";

=head2 Methods defined for the DBI class:

=head3 new Lintel::DBI( %arglist);

    my $dbh = new Lintel::DBI();
    my $dbh = new Lintel::DBI(dsn => 'DBI:mysql:test:localhost',
			      username => 'fred',
			      password => 'random',
			      application => 'myapplication');

The DSN is a database connection string as used in DBI. It defaults
to 'DBI:mysql:test:localhost'.

The username and password fields default to the values found in 
$HOME/.my.cnf, or to the username of the current user and no password
if that file is missing.

The application name defaults to the filename of the caller if none
is supplied.  

=cut

sub new {
    my($class, %arglist) = @_;

    my $dbh = Lintel::DBI->connect($arglist{dsn}, $arglist{username}, 
		                   $arglist{password}, $arglist{application});
    return $dbh;
}

=pod

=head3 Lintel::DBI->connect( $dsn, $db_username, $db_password, $application);

    my $dbh = Lintel::DBI->connect();
    my $dbh = Lintel::DBI->connect( $dsn, $db_username, $db_password, $application);

The DSN is a database connection string as used in DBI. It defaults
to 'DBI:mysql:test:localhost'.

The db_username and db_password default to the values specified in 
$HOME/.my.cnf, or to the username of the current user and no password
if that file is missing.

=cut

sub connect {
    my($class, $dsn, $db_username, $db_password, $application) = @_;

    $dsn = 'DBI:mysql:test:localhost' unless defined $dsn;
    $db_username = getpwuid($UID) unless defined $db_username;
    $application ||= (caller)[1];

    if (!defined $db_password && $dsn =~ /^DBI:mysql/o) {
	my ($name,$passwd,$uid,$gid,$quota,$comment,$gcos,
	    $dir,$shell,$expire) = getpwuid($UID);
	if (-f "$dir/.my.cnf") {
	    my $fh = new FileHandle "$dir/.my.cnf";
	    while(<$fh>) {
		if (/^\[client\]/o) {
		    while(<$fh>) {
			last if /^\[/o;
			$db_password = $1 if /^password=(\S+)\s*$/o;
		    }
		}
	    }
	}
    }

    my $dbh = DBI->connect($dsn, $db_username, $db_password, 
			   { RaiseError => 1});
    my $sth = bless { }, 'Lintel::DBI::sth';
    return bless { 'dbh' => $dbh, 
		   'sth' => $sth, 
		   'application' => $application }, $class;
}

=pod

=head3 $dbh->I<dbi_function>( $p1, ...)

   $dbh->I<dbi_function>( $p1, ...)

Lintel::DBI passes any functions it doesn't recognize on to the underlying
DBI connection.  See perldoc DBI for details.  Ensures that the DBI 
connection is open before calling.

=cut

sub AUTOLOAD {
    my $sub = $AUTOLOAD;

    (my $sthname = $sub) =~ s/.*:://;

    my $this = shift @_;

    return if $sthname eq 'DESTROY';
    confess "internal" unless defined $this->{dbh};
    my $ref = $this->{dbh};
    return $ref->$sthname(@_);
}

=pod 

=head3 $dbh->load_sths( %sql_queries);

   $dbh->load_sths(
   	I<query_name> => "I<SQL statement>",
	...);

Prepares statements for later use.  This method can be called more than
once if needed.

=cut
   
sub load_sths {
    my $this = shift;
    my %tmp = @_;

    while(my($k,$v) = each %tmp) {
	die "Invalid use of sth named '$k', reserved for internal use"
	    if $invalid_sth_names{$k};
	# TODO: delay prepare's until use in the Lintel::DBI::sth
	# AUTOLOAD and then cache.
	$this->{sth}->{$k} = $this->{dbh}->prepare($v);
	$this->{sth}->{__definitions}->{$k} = $v;
    }
}

=pod

=head3 $dbh->selectScalar( $query_name, $p1, ...);

    my $value = $dbh->selectScalar( "I<query_name>", $p1, ...);

Performs the specified query with the bound paramters $p1,..., and returns
a single scalar value, or undef.  If the result-set contains more than one
value selectScalar will die().

=cut

sub selectScalar {
    my ($this, $k, @args) = @_;
    my $sth = $this->{sth}->{$k};

    $sth->execute( @args);
    my $result = $sth->fetchall_arrayref(undef, 2);
    $sth->finish;

    if (@$result != 1) {
	if (@$result == 0) {
	    return undef;
	}
 	die( "selectScalar($k) returns ${@$result} rows (should be 1)");
    }

    my $row = $result->[0];
    if (@$row != 1) {
        die( "selectScalar($k) returns ${@$row} columns (should be 1)");
    }

    return $row->[0];
}

=pod

=head3 $dbh->beginTxn();

    $dbh->beginTxn();

Start a transaction.

=cut


sub beginTxn {
    my ($self) = @_;
    $self->{dbh}->begin_work 
	or die("Unable to start transaction: ".$self->{dbh}->errstr);
}

=pod

=head3 $dbh->commitTxn()

    $dbh->commitTxn();

Commit a transaction

=cut

sub commitTxn {
    my ($self) = @_;
    $self->{dbh}->commit
	or die("Commit failed: ".$self->{dbh}->errstr);
}

=pod

=head3 $dbh->rollbackTxn()

    $dbh->rollbackTxn();

Cancel a transaction and rollback the database

=cut

sub rollbackTxn {
    my ($self) = @_;
    $self->{dbh}->rollback;
}


=pod

=head3 $dbh->transaction( $self, \&lambda);

    $dbh->transaction( $self, \&lambda);

Executes the function $lambda surrounded by a transaction. If the lambda
function dies then transaction will rollback the transaction and reraise
the exception.

If the function completes then the transaction is committed.

Example:

    my ($user, $charge);
    $dbh->transaction( sub {
	    my $avail = $dbh->{sth}->get_money($user)->value();
	    if ($avail > $charge) {
		$dbh->{sth}->set_money($user, $avail-$charge);
		$dbh->{sth}->process_payment($user, $charge);
	    }
	});

=cut

sub transaction {
    my ($self, $function) = @_;
    $self->beginTxn();
    eval {
        &$function();
        $self->commitTxn();
    };
    if ($@) {
        $self->rollbackTxn();
	if ($@ =~ /\n$/) {
	    croak( $@);
	} else {
	    print STDERR "Error: $@\n";
            confess("transaction aborted");
	}
    }
}

=pod

=head3 $dbh->runSQL()

   $dbh->runSQL( $sql);

Runs a series of SQL commands such as from a HERE document separated by 
semicolons.  Statements can be broken over as many lines as needed. i

For example:

   $dbh->runSQL <<END;
create table users ( id integer primary key, name char(20));
create table email ( user_id integer, address char(128));
create index email_idx1 on email (user_id);
END

=cut

sub runSQL {
    my ($self, $sql) = @_;
    my @lines = split( /\n/, $sql);
    my $statement = "";
    foreach (@lines) {
        $statement .= "$_\n";
	if (/;/) {
	    $self->{dbh}->do( $statement);
	    $statement = "";
	}
    }
}

=pod

=head3 $dbh->setConfig( $key, $value)
   
    $dbh->setConfig( $key, $value);

Writes a key and value (both strings) to the dbi_config table. The dbi_config
table is created automatically when using the loadSchema() method, and is
used here to store the schema version number for version checking and migration.

Client applications are also welcome to use this table but should avoid 
setting keys that match the application source filename.

=cut

sub setConfig {
    my ($self, $name, $value) = @_;
    if (! defined($self->{setconfig})) {
	$self->{setconfig} = $self->{dbh}->prepare("replace into dbi_config values ( ?, ?)");
    }
    $self->{setconfig}->execute( $name, $value);
}


=pod

=head3 $dbh->getConfig( $key)

    my $value = $dbh->getConfig( $key);
   
Returns the value of the config key specified, or undef if none is defined.

=cut

sub getConfig {
    my ($self, $name) = @_;
    if (!defined($self->{getconfig})) {
	$self->{getconfig} = $self->{dbh}->prepare("select value from dbi_config where name = ?");
    }
    $self->{getconfig}->execute( $name);
    my $rv = $self->{getconfig}->fetchall_arrayref();
    if (@$rv != 1) {
	croak("Ambiguous primary key");
    }
    return (defined($rv->[0]) && $rv->[0]->[0]) or undef;
}

=pod

=head3 $dbh->loadSchema( $version, $schemaSQL, $option)

    $dbh->loadSchema( $version, $schemaSQL, $option);

Loads a schema with the specified schema version by applying the specified
SQL statements.  

To actually load the schema the value for $option must be set to "--init".
This is a safety feature to ensure that the loadSchema method is only 
invoked when required.

For example:

    $dbh->loadSchema( 1, <<END, "--init");
create table users ( id integer primary key, name char(20));
create table email ( user_id integer, address char(128));
create index email_idx1 on email (user_id);
END
   
=cut

sub loadSchema {
    my ($self, $schemaVersion, $schema, $option) = @_;

    if ($option eq "--init") {
	$self->runSQL( <<END);
create table if not exists dbi_config (
	name varchar(32) primary key,
	value varchar(1024)
);
END
	$self->transaction( sub {
		$self->runSQL( $schema);
		$self->setConfig( $self->{application}, $schemaVersion);
	    });
    }
}

=pod

=head3 $dbh->migrateSchema( $targetVersion, \%migration);

   $dbh->migrateSchema( $targetVersion, \%migration);

Migrate Schema from whatever is current to the version specified in 
$targetVersion.  The $migration parameter is a hash reference to a
set of schema update statements.  Upgrade statements can (but do not 
have to) include multiple lines.

For example:

   $dbh->migrateSchema( 3, { 
    	1 => { to_version => 2,
    	       sql => 'alter table foo add column name char(256);' },
    	2 => { to_version => 3, 
    	       sql => 'drop table bar;' }
    	}); 

=cut

sub migrateSchema {
    my ($self, $schemaVersion, $migration) = @_;
    my $dbVersion = $self->getActualVersion();

    while ($dbVersion < $schemaVersion) {
        if (defined $migration->{$dbVersion}) {
	    $self->transaction( sub {
		    $self->runSQL($migration->{$dbVersion}->{sql});
		    $self->setConfig( 
			    $self->{application},
			    $migration->{$dbVersion}->{to_version}
			);
		});
	    $dbVersion = $migration->{$dbVersion}->{to_version}
	} else {
	    croak "There is no defined upgrade path from version $dbVersion";
	}
    }

    print "Database migrated\n";
    $self->checkVersion( $schemaVersion);
}


sub getActualVersion { 
    my ($self) = @_;
    my $version;

    eval {
	$version = $self->getConfig($self->{application});
    };

    return $version;
}

=pod

=head3 $dbh->checkVersion( $schemaVersion);

   $dbh->checkVersion( $schemaVersion);

Verifies that the specified schema version is loaded, and prints an 
error message if the application doesn't match the database.

=cut

sub checkVersion {
    my ($self, $schemaVersion) = @_;

    my $version = $self->getActualVersion();

    if (!defined $version) {
	croak("The schema version is missing.  Has the schema been installed?");
    }

    if ($version < $schemaVersion) {
	croak("The schema version $version is out of date");
    } elsif ($version > $schemaVersion) {
	croak("This program is too old to work with schema version $version");
    }
}

package Lintel::DBI::sth;

=pod

=head2 Functions on Lintel::DBI::sth

Functions defined on the Lintel::DBI::sth class as returned by 
accessing the database handle's {sth} member variable.  Typical
usage is shown below.

=cut

# if you add functions to this package, add them to the list of
# invalid sth names at the top of the file.

use AutoLoader;
use vars '$AUTOLOAD';
use Carp;
use strict;

=pod

=head3 $dbh->{sth}->I<query_name>( $p1, ...);

    my $sth = $dbh->{sth}->I<query_name>( $p1, ...);

The sth member maps to a SQL query that is preloaded using the 
$dbh->load_sths() method.  The parameters $p1 and so on are bound
to the parameters of the query.  

This does the DBI equivalent of an execute().  The return value
is of type Lintel::DBI::exec_sth() which can be used for further
operations to retrieve the execution results.

=cut

sub AUTOLOAD {
    my $sub = $AUTOLOAD;

    (my $sthname = $sub) =~ s/.*:://;

    my $this = shift @_;

    die "unknown SQL sth $sthname" unless defined $this->{$sthname};
    my $sth = $this->{$sthname};
    $this->{__laststh} = $sth;

    my $exec_ret = eval { $sth->execute(@_) };
    confess "Unable to execute SQL '$this->{__definitions}->{$sthname}'; args=(" . join(", ", @_) . "): $@"
	if $@;
    return bless { 
	'sth' => $sth,
	'exec_ret' => $exec_ret,
    }, 'Lintel::DBI::exec_sth';
}

sub DESTROY {
    confess "internal" unless ref $_[0] && ! defined $_[0]->{DESTROY};
}

sub fetchArray {
    my ($this) = @_;

    return $this->{__laststh}->fetchrow_array();
}

sub fetchHash {
    my ($this) = @_;

    return $this->{__laststh}->fetchrow_hashref();
}


sub oneRow {
    my $this = shift @_;

    if (@_ > 0) {
	delete $this->{__laststh};
	my $sth = shift @_;
	$this->$sth(@_);
    } else {
	die "Did not execute a statement??"
	    unless defined $this->{__laststh};
    }

    return $this->{__laststh}->oneRow();
}

sub atMostOneRow {
    my $this = shift @_;

    if (@_ > 0) {
	delete $this->{__laststh};
	my $sth = shift @_;
	$this->$sth(@_);
    } else {
	die "Did not execute a statement??"
	    unless defined $this->{__laststh};
    }

    return $this->{__laststh}->atMostOneRow();
}
    

package DBI::st;

# TODO: PURGE all of this (assuming we can) after the fixup of
# manage-network-config, remote-device-operations, etc. which may be
# using this.  The exec_sth style is a better choice.

use Carp;
use strict;

sub execOneRow {
    my($sth, @args) = @_;

    $sth->execute(@args)
	or die "Execute of $sth failed";

    return $sth->oneRow();
}

sub oneRow {
    my($sth) = @_;

    my $ret = eval { $sth->fetchrow_hashref(); };
    confess $@ if $@;
    confess "?? $sth" unless defined $ret;
    my $tmp = $sth->fetchrow_hashref();
    confess "?? $tmp" if defined $tmp;
    return $ret;
}

sub oneRowArray {
    my($sth) = @_;

    my $ret = $sth->fetchrow_array();
    confess "?? $sth" unless defined $ret;
    my $tmp = $sth->fetchrow_array();
    confess "?? $tmp" if defined $tmp;
    return @$ret;
}

sub execAtMostOneRow {
    my($sth, @args) = @_;

    $sth->execute(@args)
	or die "Execute of $sth failed";

    return $sth->atMostOneRow();
}

sub atMostOneRow {
    my($sth) = @_;

    my $ret = $sth->fetchrow_hashref();
    return undef unless defined $ret;
    my $tmp = $sth->fetchrow_hashref();
    confess "?? $tmp" if defined $tmp;
    return $ret;
}

sub atMostOneRowArray {
    my($sth) = @_;

    my @ret = $sth->fetchrow_array();
    return () unless @ret;
    my @tmp = $sth->fetchrow_array();
    confess "?? @tmp" if @tmp > 0;
    return @ret;
}

package Lintel::DBI::exec_sth;
=pod

=head2 Functions on Lintel::DBI::exec_sth

Operations on the Lintel::DBI::exec_sth class as returned from executing
a query on the database handle.  See also Lintel::DBI::sth above.

=cut

use strict;

=pod

=head3 $xs->result();

    my $xs = $dbh->{sth}->myquery( $p1, ...);
    my $result = $xs->result();

Returns the single output from a query.  Returns undef if the query
returned nothing.  If the result set contains more than one row or 
column die().

=cut

sub result {
    my($this) = @_;

    die "Can't call result() unless result of execution was successful"
	unless $this->{exec_ret};
    my @result =  $this->{sth}->atMostOneRowArray();
    if (@result > 1) {
	die "Can't use multi-column results with result()";
    }
    return $result[0];
}

=pod

=head3 $xs->fetchrow_array();

    my $xs = $dbh->{sth}->myquery( $p1, ...);
    my $hashref = $xs->oneRow();

Returns the next row of query result as hash reference.  Results are filled
into the hash key with names matching the output columns from the query. 
The oneRow() function ensures that exactly one row is returned by the query
and will die() if that is not the case.

For example:

    $dbh->load_sths( total_price => "select sum(price) as total from order where order_number = ?");
    my $result = $dbh->{sth}->total_price( $order_number)->oneRow();
    printf "Total is: %d\n", $result->{price};

=cut
sub oneRow {
    my($this) = @_;

    die "Can't call oneRow unless result of execution was successful"
	unless $this->{exec_ret};
    return $this->{sth}->oneRow();
}

=pod

=head3 $xs->oneRowArray();

    my $xs = $dbh->{sth}->myquery( $p1, ...);
    my @result = $xs->oneRowArray();

Returns the next row of query result as an array of values.  Results are 
filled into the array in the order that the results are listed in the SQL
select statement.  The oneRowArray() function ensures that exactly one 
row is returned by the query and will die() if that is not the case.

=cut
sub oneRowArray {
    my($this) = @_;

    die "Can't call oneRowArray unless result of execution was successful"
	unless $this->{exec_ret};
    return $this->{sth}->oneRowArray();
}

=pod

=head3 $xs->atMostOneRow();

    my $xs = $dbh->{sth}->myquery( $p1, ...);
    my $result = $xs->atMostOneRow();

Returns the next row of query result as a hash reference.  If there
is no next row then atMostOneRow() returns undef.
The oneRowArray() function ensures that either 1 or 0 rows are
returned by the query and will die() if that is not the case.

For example:

    $dbh->load_sths( margin => "select margin from inventory where sku = ?");
    if (my $result = $dbh->{sth}->margin($sku)->atMostOneRow()) {
	printf "Margin for %s is: %d\n", $sku, $result->{max_price};
    }

=cut
sub atMostOneRow {
    my($this) = @_;

    die "Can't call atMostOneRow unless result of execution was successful"
	unless $this->{exec_ret};
    return $this->{sth}->atMostOneRow();
}

=pod

=head3 $xs->atMostOneRowArray();

    my $xs = $dbh->{sth}->myquery( $p1, ...);
    my @result = $xs->atMostOneRowArray();

Returns the next row of query result as an array.  If there
is no next row then atMostOneRowArray() returns an empty array.
The oneRowArray() function ensures that either 1 or 0 rows are
returned by the query and will die() if that is not the case.

=cut
sub atMostOneRowArray {
    my($this) = @_;

    die "Can't call atMostOneRowArray unless result of execution was successful"
	unless $this->{exec_ret};
    return $this->{sth}->atMostOneRowArray();
}

=pod

=head3 $xs->fetchHash()

   my $xs = $dbh->{sth}->myquery( $p1,...);
   my $row = $xs->fetchHash();

Returns the next row of the query result as a hash.  If there are no more
rows return undef.

For example:

   my $dbh->load_sths( inventory => "select sku, count, desc from inventory");
   my $xs = $dbh->{sth}->inventory();
   while (my $row = $xs->fetchHash()) {
      print "%s, %d, %s\n", $row->{sku}, $row->{count}, $row->{desc};
   }

=cut
sub fetchHash {
    my($this) = @_;

    die "Can't call fetchHash unless result of execution was successful"
	unless $this->{exec_ret};
    return $this->{sth}->fetchrow_hashref();
}

=pod

=head3 $xs->fetchArray();

    my $xs = $dbh->{sth}->myquery( $p1,...);
    my @row = $xs->fetchArray();

Returns the next row of the result set as an array.  If there is no next
row returns an empty array.

=cut

sub fetchArray {
    my($this) = @_;

    die "Can't call fetchArray unless result of execution was successful"
	unless $this->{exec_ret};
    return $this->{sth}->fetchrow_array();
}
   
1;

package Lintel::DBI; # get an error from AutoSplit.pm otherwise
__END__

