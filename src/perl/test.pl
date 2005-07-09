# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

######################### We start with some black magic to print on failure.

# Change 1..1 below to 1..last_test_to_print .
# (It may become useful if the test is moved to ./t subdirectory.)

BEGIN { $| = 1; print "1..1\n"; }
END {print "not ok 1\n" unless $loaded;}
use Lintel;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Insert your test code below (better if it prints "ok 13"
# (correspondingly "not ok 13") depending on the success of chunk 13
# of the test code):

my $stats = new Lintel::Histogram::Log(10,1,15);

print "Hi $stats\n";
print "Add...\n";
$stats->add(5);
print "Count...\n";
my $c = $stats->count();

print "You should see: Hi $stats 1.\n";
print "Output:         Hi $stats $c.\n";

print "You should see '1' in the bin containing 5.\n";
for($i=0;$i<$stats->numBins();++$i) {
    printf ("    [%5.2f .. %5.2f] %ld, avg %.2f\n", $stats->binlow($i),
	    $stats->binhigh($i), $stats->binCount($i), $stats->binMean($i));
}

$stats = new Lintel::Stats();
$stats->add(5);
$stats->add(5);
$stats->add(6);
$stats->add(7);
$stats->add(3);
print "Count = ", $stats->count(), " (should be 5).\n";
print "Min = ", $stats->min(), " (should be 3).\n";
print "Max = ", $stats->max(), " (should be 7).\n";
print "Mean = ", $stats->mean(), " (should be 5.2).\n";
print "conf95 = ", $stats->conf95(), " (should be 1.6...).\n";


