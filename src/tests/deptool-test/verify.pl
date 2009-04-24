#!/usr/bin/perl
package Main;
use Carp;
use Digest::MD5;

our $obj='build/opt-debian-i686';
our $src='projects';

my $err = verify($ARGV[0]);
exit( $err);

package Test;
use Data::Dumper;

sub new {
    my ($class, $test) = @_;
    my $self = {};
    if ($test =~ /(\S+)\s*(>|==|#=)\s*(\S+)/) {
	$self->{f1}=expand($1);
	$self->{op}=$2;
	$self->{f2}=expand($3);
    } elsif ($test =~ /(-d)\s+(\S+)/) {
	$self->{op}=$1;
	$self->{f1}=expand($2);
    }
    return bless $self, $class;
}

sub expand {
    my ($simple) = @_;
    my $path;
    if ($simple =~ m!OBJ(\{[^}]+\})?/([^/]+)/(\w+)_stamp!) {
	$path = "$Main::obj$1/$2/.deptool.$3_stamp";
    } else {
	$path = $simple;
	$path =~ s/OBJ/$Main::obj/;
	$path =~ s/SRC/$Main::src/;
    }
    return $path;
}

sub mtime {
    my ($self, $op) = @_;
    my $ok=0;

    my $f1mtime = (-M $self->{f1});
    my $f2mtime = (-M $self->{f2});

    if ($op eq ">") {
	$ok = ($f1mtime <= $f2mtime);
    } elsif ($op eq "==") {
	$ok = ($f1mtime == $f2mtime);
    }

    if (!$ok) {
	print "-M $self->{f1} = ",((-M $self->{f1}) * 86400.0),"\n";
	print "-M $self->{f2} = ",((-M $self->{f2}) * 86400.0),"\n";
    }
    return $ok;
}

sub hashMatch {
    my ($self) = @_;
    my $ok;

    my $h1 = new Digest::MD5();
    open my $f1, $self->{f1};
    $h1->addfile( $f1);
    close $f1;
    my $h1hex = $h1->hexdigest();

    my $h2 = new Digest::MD5();
    open my $f2, $self->{f2};
    $h2->addfile( $f2);
    close $f2;
    my $h2hex = $h2->hexdigest();

    $ok = ($h1hex eq $h2hex);
    if (!$ok) {
	print "digest $self->{f1} = $h1hex\n";
	print "digest $self->{f2} = $h2hex\n";
    }
    return $ok;
}

sub dfile {
    my ($self) = @_;
    my $ok = 1;
    if (!-f $self->{f1} || !-f $self->{f2}) {
	$ok = 0;
	print "Missing file $self->{f1}\n" if (!-f $self->{f1});
	print "Missing file $self->{f2}\n" if (!-f $self->{f2});
    } 
    return $ok;
}

sub result {
    my ($self) = @_;
    my $op = $self->{op};
    my $res = 0;

    if ($op eq '==' || $op eq '>') {
	if ($self->dfile()) {
	    $res = $self->mtime( $op);
	} 
    } elsif ($op eq '#=') {
	if ($self->dfile()) {
	    $res = $self->hashMatch();
	}
    } elsif ($op eq '-d') {
	$res = (-d $self->{f1});
    } else {
	print "Unknown op $op\n";
	$res = 0;
    }
    return $res;
}

package Main;
sub runtests {
    my $err = 0;
    my $line = 1;
    foreach (@_) {
	my $test = new Test($_);
        if (!$test->result()) {
	    $err++;
	    print STDERR "$line: Failed in $_\n";
	}
	$line ++;
    }
    return $err;
}

sub verify {
    my ($realtest, $offset) = @_;
    my $err = 0;
    $offset ||= 0;

    my $test = $realtest+$offset;

    if ($test == 1) {
	$err += runtests ("-d SRC/a",
			  "-d SRC/b",
			  "-d SRC/c");
    } elsif ($test == 2) {

	$err += runtests(
		         #build A
		         "OBJ/a/build_stamp > SRC/a/a.cpp",
			 "OBJ/a/build_stamp > SRC/a/a.h",
			 "OBJ/a/liba.a > SRC/a/a.h",
			 "OBJ/a/liba.a > SRC/a/a.cpp",
			 "OBJ/a/install_stamp > OBJ/lib/liba.a",
			 "OBJ/a/install_stamp > OBJ/include/a.h",
			 "OBJ/lib/liba.a > OBJ/a/liba.a",
			 "OBJ/include/a.h #= SRC/a/a.h",

			 #build B
			 "OBJ/b-ANSIColor/build_stamp > SRC/b/b.h",
			 "OBJ/b-ANSIColor/build_stamp > SRC/b/b.cpp",
			 "OBJ/b-ANSIColor/libb-ANSIColor.a > SRC/b/b.h",
			 "OBJ/b-ANSIColor/libb-ANSIColor.a > SRC/b/b.cpp",

			 "OBJ/b-ANSIColor/build_stamp > OBJ/a/install_stamp",
			 "OBJ/b-ANSIColor/install_stamp > OBJ/lib/libb-ANSIColor.a",
			 "OBJ/b-ANSIColor/install_stamp > OBJ/include/b.h",
			 "OBJ/lib/libb-ANSIColor.a > OBJ/b-ANSIColor/libb-ANSIColor.a",
			 "OBJ/include/b.h > SRC/b/b.h",
			 
			 "OBJ/b/build_stamp > SRC/b/b.h",
			 "OBJ/b/build_stamp > SRC/b/b.cpp",
			 "OBJ/b/libb-ASCII.a > SRC/b/b.h",
			 "OBJ/b/libb-ASCII.a > SRC/b/b.cpp",

			 "OBJ/b/build_stamp > OBJ/a/install_stamp",
			 "OBJ/b/install_stamp > OBJ/lib/libb-ASCII.a",
			 "OBJ/b/install_stamp > OBJ/include/b.h",
			 "OBJ/lib/libb-ASCII.a > OBJ/b/libb-ASCII.a",
			 "OBJ/include/b.h > SRC/b/b.h",

			 "OBJ/b/install_stamp > OBJ/b-ANSIColor/install_stamp",

			 # build C
			 "OBJ/b-HTML/build_stamp > OBJ/b/install_stamp",
			 "OBJ/b-HTML/install_stamp > OBJ/b-HTML/build_stamp",
			 "OBJ/lib/libb-HTML.a > OBJ/b-HTML/libb-HTML.a",

			 "OBJ/c-HTML/build_stamp > OBJ/a/install_stamp",
			 "OBJ/c-HTML/build_stamp > OBJ/b/install_stamp",
			 "OBJ/c-HTML/build_stamp > OBJ/lib/libb-HTML.a",
			 "OBJ/c-HTML/build_stamp > OBJ/include/b.h",
			 "OBJ/c-HTML/build_stamp > SRC/c/main.cpp",

			 "OBJ/c-HTML/install_stamp > OBJ/c-HTML/build_stamp",
			 "OBJ/c-HTML/c-HTML > OBJ/c-HTML/build_stamp",
#			 "OBJ/bin/c-HTML > OBJ/c-HTML/c-HTML",

			 "OBJ/c/build_stamp > OBJ/a/install_stamp",
			 "OBJ/c/build_stamp > OBJ/b/install_stamp",
			 "OBJ/c/build_stamp > OBJ/lib/libb-ASCII.a",
			 "OBJ/c/build_stamp > OBJ/include/b.h",
			 "OBJ/c/build_stamp > SRC/c/main.cpp",

			 "OBJ/c/install_stamp > OBJ/c/build_stamp",
			 "OBJ/c/c > OBJ/c/build_stamp",
#			 "OBJ/bin/c > OBJ/c/c"
			);
    } elsif ($test == 3) {
	$err += verify(2);
	my $t3 = $realtest;
	my $t2 = $realtest-1;
	$err += runtests("OBJ{test$t3}/a/build_stamp > OBJ{test$t2}/c/install_stamp");
    } elsif ($test >= 9 && $test <= 15) {
	$err += verify($test, $offset-9+2);
	$err += runtests("OBJ/a/test_stamp > OBJ/a/install_stamp",
		         "OBJ/b/test_stamp > OBJ/b/install_stamp",
			 "OBJ/c/test_stamp > OBJ/c/install_stamp");
    } elsif ($test >= 16 && $test <= 30) {
	$obj = "build/dbg-debian-i686";
	$err += verify($test, -16+2);
    } else {
	print "Unimplemented test $test\n";
	$err++;
    }

    return $err;
}

