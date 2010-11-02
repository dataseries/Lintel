#!/usr/bin/perl -w
use strict;
use Time::HiRes 'time';
use Lintel::ProcessManager;

my $process_manager = new Lintel::ProcessManager();

test(1);
test(4);
test(16);
test(64);

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

exit(0);


