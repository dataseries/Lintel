#!/usr/bin/perl -w
use strict;
use FileHandle;
use Lintel::ProcessManager 'maskCoredump';
use Time::HiRes 'time';

my $process_manager = new Lintel::ProcessManager();

$process_manager->{debug} = 1;
eval "use BSD::Resource";
unless ($@) {
    testCoreDump();

    memtest(1024*1024, 0);
    memtest(100*1024*1024, 1);
}
test(1);
test(4);
test(16);
test(64);


sub memtest {
    my ($objsize, $expect_error) = @_;
    print "Expect an \"Out of memory\" error to be printed next, verifying it is caught\n"
        if $expect_error;
    my $pid = $process_manager->fork(
	cmd => "perl -e 'BEGIN { my \$foo = \".\"x$objsize; } exit(0);'",
        # limit of 25 MB does not work on centos5 chroot (1 MB size does not succeed)
        # Note on 10/19/2012: doubled max_mem_bytes because otherwise (on Centos) there's 
	# error:  "41 Testing lintel-processmanager         ***Failed  27/"
	# max_mem_bytes => 50*1024*1024); 
	 max_mem_bytes => 2*50*1024*1024); 
    my $ecode = $process_manager->waitPid($pid);
    die "unexpected error $ecode on memtest $objsize" if $ecode && !$expect_error;
    die "unexpected completion $ecode on memtest $objsize" if !$ecode && $expect_error;

    print "Memory test passed\n";
}

sub test {
    my ($procs_per_wait) = @_;

    for (my $tries = 0; $tries < 3; ++$tries) {
        my $start = time();

        my $forked = 0;
        while (time() < $start + 1) {
            for my $i (1 .. $procs_per_wait) {
                $process_manager->fork(cmd => sub { exit(0); },
                                       exitfn => sub { ++$forked; });
            }
            $process_manager->waitAll();
        }

        my $end = time();

        my $elapsed = $end - $start;
        my $rate = $forked / $elapsed;

        printf "$procs_per_wait processes/wait, forked %d in %.2fs, %.1f/s\n", 
            $forked, $elapsed, $rate;
        # Earlier bug in processmanager led to ~10/s
        return if $rate > 25; # RHEL5 only gets about 270/s
        warn "Fork rate abnormally low for try $tries; sleep(5)";
        sleep(5);
    }
}

sub testCoreDump {
    if (-f "core") {
        unlink("core");
    }
    eval q{ setrlimit(RLIMIT_CORE, 0, 4096) or die "setrlimit: $!"; }; die $@ if $@;
    my $pid = $process_manager->fork(cmd => sub { kill 'SEGV', $$ });
    my $status = $process_manager->waitPid($pid);
    print `kill -l $status`;

    die "? $status on $pid" unless `kill -l $status` eq "SEGV\n";
    $status = 11; # the status was SEGV, but not necessarily 11; see posix spec

    eval q{ setrlimit(RLIMIT_CORE, 4096, 4096) or die "setrlimit(core-4k): $!"; }; die $@ if $@;

    my $do_status_test = 1;
    if (-f "/etc/SuSE-release") {
        # openSUSE 11.2 does not properly return the core status.  Tested using a simple
        # program that calls system("thing that aborts").  The core file is generated, the
        # status is wrong.  The C equivalent works properly.
        my $fh = new FileHandle "/etc/SuSE-release" or die "?";
        $_ = <$fh>;
        $do_status_test = 0 if / 11\.2 /o;
    }

    $pid = $process_manager->fork(cmd => sub { kill 'SEGV', $$ });
    my $core_status = $process_manager->waitPid($pid);

    if ($do_status_test) {
        die "? $core_status" unless $core_status == (11 | 0x80);

        my $mask_status = maskCoredump($core_status);
        die "? $mask_status != $status" unless $mask_status == $status;
    } else {
        warn "Skipping core status test, it is broken on this platform; see test-lintel-processmanager for details";
    }

    die "missing core file" unless -f "core" || -f "core.$pid";
    unlink("core", "core.$pid");
    
    print "Core dump masking test passed\n";
}

exit(0);


