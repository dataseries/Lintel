#
#  (c) Copyright 2009, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl test.pl'

# TODO: rewrite this using Test::More; when doing so, make tests store
# values as an array, join it together, run it through splitSql and
# verify the same thing comes back.

######################### We start with some black magic to print on failure.

# Change 1..1 below to 1..last_test_to_print .
# (It may become useful if the test is moved to ./t subdirectory.)

BEGIN { $| = 1; print "1..3\n"; }
END {print "not ok 1\n" unless $loaded;}
use Lintel::DBI;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Insert your test code below (better if it prints "ok 13"
# (correspondingly "not ok 13") depending on the success of chunk 13
# of the test code):

use Carp;

my $err = 0;
$err ||= test2();
$err ||= test3();
exit($err);

sub test2 {
    my $sql = <<END;
-- create table statements schema version 10

create table scrum_task (
  title varchar(255),
  created date not null,
  estimated_days double not null,
  finished date,
  actual_days double not null,
  sprint int(11) not null,
  state enum('available', 'work', 'forreview', 'review', 'canceled', 'incomplete', 'done', 'meta') default 'available',
  assigned_to varchar(255),
  assigned_date date,
  for_review date,
  project varchar(255),
  review_reference varchar(255),
  primary_reviewer varchar(255),
  id varchar(4) not null,
  parent varchar(4),
  primary key (id,sprint)
);

create table task_backup like scrum_task;

create table sprint_date (
  sprint int(11) default NULL,
  start_date date default NULL
);

create table config (
    name varchar(32) primary key,
    value varchar(512)
);

create table work_log (
    sprint integer,
    id varchar(4),
    username varchar(32) not null,
    work double not null
);

create table review_log (
    sprint integer,
    id varchar(4),
    review date,
    review_reference varchar(255),
    outcome enum('accept', 'reject')
);

insert into config values 
    ( 'schema_version', '$currentSchemaVersion');

END

    my $statements = Lintel::DBI::splitSQL($sql);
    if (@$statements != 7) {
	print "not ok 2\n";
	return 1;
    } 
    print "ok 2\n";
    return 0;
}

sub test3 {
    my $sql = <<END;
-- this is a comment with a ; embedded
select * from receipts where name = 'O''Brien';

-- this is a comment with '; embedded
select count(*) from filelist where filename like "%;2";

-- this is a commenct with ;" embedded
select sum(price) 
    from neworder 	-- the new order table; ks1 2009-04-01
    where buyer = ?
    group by ponumber
    order by date;

-- this is a comment with ;'" embedded
select name || ';' || number
    from employees;

END

    my $obj = bless {}, 'Lintel::DBI';
    my $statements = $obj->splitSQL($sql);
    if (@$statements != 4) {
	print "not ok 3\n";
	return 1;
    } 
    print "ok 3\n";
    return 0;
}
