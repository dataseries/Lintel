/* -*-C++-*-
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <boost/tuple/tuple_io.hpp>
#include <Lintel/LintelLog.hpp>
#include <Lintel/StatsCube.hpp>
#include <Lintel/StringUtil.hpp>

using namespace std;
using boost::format;

typedef boost::tuple<int32_t, bool, string> IBSTuple;
typedef lintel::StatsCube<IBSTuple> IBSCube;

void checkWalk(const IBSCube::MyAny &ptuple, Stats &stat) {
    if (!ptuple.any[0]) {
	SINVARIANT(stat.count() == 1);
	SINVARIANT(stat.mean() == ptuple.data.get<0>() + 1);
    } else { // can have merges; (*, ? , ?)
	if (ptuple.any[1] && ptuple.any[2]) { // complete merge: (*, *, *) 
	    SINVARIANT(stat.count() == 3 && stat.mean() == 2);
	} else if (ptuple.any[1]) { // partial merge: (*, *, v) 
	    if (ptuple.data.get<0>() == 1) { // singleton
		SINVARIANT(stat.count() == 1 && stat.mean() == 2);
	    } else {
		SINVARIANT(stat.count() == 2 && stat.mean() == 2);
	    }
	} else if (ptuple.any[2]) { // partial merge: (*, v, *)
	    if (ptuple.data.get<0>() == 0) { // singleton
		SINVARIANT(stat.count() == 1 && stat.mean() == 1);
	    } else {
		SINVARIANT(stat.count() == 2 && stat.mean() == 2.5);
	    }
	} else { // partial merge: (*, v, v)
	    SINVARIANT(stat.count() == 1 && stat.mean() == ptuple.data.get<0>() + 1);
	}
    }
}

static IBSCube::MyAny cwo_last_tuple(IBSTuple(-1, true, "a"));

void checkWalkOrdered(const IBSCube::MyAny &ptuple, Stats &stat) {
    SINVARIANT(cwo_last_tuple < ptuple);
    cwo_last_tuple = ptuple;
    checkWalk(ptuple, stat);
}

struct ZeroData {
    string key;
    uint32_t count;
    uint32_t sum;
};

// Both of the below calculated using the perl program below; first with zero_count = 0,
// second with zero_count = 1
ZeroData raw_data_a[] = {
 { "*,*,*", 3, 6 }, { "*,*,a", 2, 4 }, { "*,*,b", 1, 2 }, { "*,f,*", 2, 5 }, { "*,f,a", 1, 3 },
 { "*,f,b", 1, 2 }, { "*,t,*", 1, 1 }, { "*,t,a", 1, 1 }, { "*,t,b", 0, 0 }, { "0,*,*", 1, 1 },
 { "0,*,a", 1, 1 }, { "0,*,b", 0, 0 }, { "0,f,*", 0, 0 }, { "0,f,a", 0, 0 }, { "0,f,b", 0, 0 },
 { "0,t,*", 1, 1 }, { "0,t,a", 1, 1 }, { "0,t,b", 0, 0 }, { "1,*,*", 1, 2 }, { "1,*,a", 0, 0 },
 { "1,*,b", 1, 2 }, { "1,f,*", 1, 2 }, { "1,f,a", 0, 0 }, { "1,f,b", 1, 2 }, { "1,t,*", 0, 0 },
 { "1,t,a", 0, 0 }, { "1,t,b", 0, 0 }, { "2,*,*", 1, 3 }, { "2,*,a", 1, 3 }, { "2,*,b", 0, 0 },
 { "2,f,*", 1, 3 }, { "2,f,a", 1, 3 }, { "2,f,b", 0, 0 }, { "2,t,*", 0, 0 }, { "2,t,a", 0, 0 },
 { "2,t,b", 0, 0 },
};

uint32_t nraw_data_a = sizeof(raw_data_a)/sizeof(ZeroData);

HashMap< string, std::pair<uint32_t, uint32_t> > zero_data;

ZeroData raw_data_b[] = {
 { "*,*,*", 12, 6 }, { "*,*,a", 6, 4 }, { "*,*,b", 6, 2 }, { "*,f,*", 6, 5 }, { "*,f,a", 3, 3 },
 { "*,f,b", 3, 2 }, { "*,t,*", 6, 1 }, { "*,t,a", 3, 1 }, { "*,t,b", 3, 0 }, { "0,*,*", 4, 1 },
 { "0,*,a", 2, 1 }, { "0,*,b", 2, 0 }, { "0,f,*", 2, 0 }, { "0,f,a", 1, 0 }, { "0,f,b", 1, 0 },
 { "0,t,*", 2, 1 }, { "0,t,a", 1, 1 }, { "0,t,b", 1, 0 }, { "1,*,*", 4, 2 }, { "1,*,a", 2, 0 },
 { "1,*,b", 2, 2 }, { "1,f,*", 2, 2 }, { "1,f,a", 1, 0 }, { "1,f,b", 1, 2 }, { "1,t,*", 2, 0 },
 { "1,t,a", 1, 0 }, { "1,t,b", 1, 0 }, { "2,*,*", 4, 3 }, { "2,*,a", 2, 3 }, { "2,*,b", 2, 0 },
 { "2,f,*", 2, 3 }, { "2,f,a", 1, 3 }, { "2,f,b", 1, 0 }, { "2,t,*", 2, 0 }, { "2,t,a", 1, 0 },
 { "2,t,b", 1, 0 },
};

uint32_t nraw_data_b = sizeof(raw_data_b)/sizeof(ZeroData);

void loadZeroData(ZeroData *raw_data, uint32_t nraw_data) {
    for(uint32_t i = 0; i < nraw_data; ++i) {
	ZeroData &z = raw_data[i];
	zero_data[z.key] = std::pair<uint32_t, uint32_t>(z.sum, z.count);
    }
}

void checkWalkZero(const IBSCube::MyAny &ptuple, Stats &stat) {
    vector<string> tmp;
    tmp.push_back(ptuple.any[0] ? "*" : str(format("%d") % ptuple.data.get<0>()));
    tmp.push_back(ptuple.any[1] ? "*" : (ptuple.data.get<1>() ? "t" : "f"));
    tmp.push_back(ptuple.any[2] ? "*" : ptuple.data.get<2>());

    string key = join(",", tmp);
    SINVARIANT(zero_data.exists(key));
    INVARIANT(zero_data[key].second == stat.count(), format("%s: %d != %d")
	      % key % zero_data[key].second % stat.count());
    SINVARIANT(round(stat.count() * stat.mean()) == zero_data[key].first);
    
    zero_data.remove(key);
}

void checkWalkZeroOrdered(const IBSCube::MyAny &ptuple, Stats &stat) {
    SINVARIANT(cwo_last_tuple < ptuple);
    cwo_last_tuple = ptuple;
    checkWalkZero(ptuple, stat);
}

bool pruneNoAny(const IBSCube::MyAny &ptuple) {
    LintelLogDebug("pna", boost::format("prune %s %d") % ptuple % ptuple.any.none());
    return ptuple.any.none();
}

int main() {
    LintelLog::parseEnv();
    lintel::StatsCube<IBSTuple> cube(boost::bind(&lintel::StatsCubeFns::cubeAll));

    cube.add(IBSTuple(0, true, "a"), 1);
    cube.add(IBSTuple(1, false, "b"), 2);
    cube.add(IBSTuple(2, false, "a"), 3);

    if (true) {
	cube.cube();
	
	cube.walk(boost::bind(checkWalk, _1, _2));
	cube.walkOrdered(boost::bind(checkWalkOrdered, _1, _2));

	cube.clear(false);
    }

    if (true) {
	cube.zeroCube(false);
	loadZeroData(raw_data_a, nraw_data_a);
	cube.walk(boost::bind(checkWalkZero, _1, _2));
	SINVARIANT(zero_data.size() == 0);

	cwo_last_tuple = IBSCube::MyAny(IBSTuple(-1, true, "a"));
	loadZeroData(raw_data_a, nraw_data_a);
	cube.walkOrdered(boost::bind(checkWalkZeroOrdered, _1, _2));
	SINVARIANT(zero_data.size() == 0);
	
	SINVARIANT(cube.size() == nraw_data_a);
	cube.prune(boost::bind(&pruneNoAny, _1));
	SINVARIANT(cube.size() == nraw_data_a - 3*2*2);
	cube.clear(false);
    }

    if (true) {
	cube.zeroCube(true);
	loadZeroData(raw_data_b, nraw_data_b);
	cube.walk(boost::bind(checkWalkZero, _1, _2));
	SINVARIANT(zero_data.size() == 0);

	cwo_last_tuple = IBSCube::MyAny(IBSTuple(-1, true, "a"));
	loadZeroData(raw_data_b, nraw_data_b);
	cube.walkOrdered(boost::bind(checkWalkZeroOrdered, _1, _2));
	SINVARIANT(zero_data.size() == 0);

	cube.clear(false);
    }

    cube.clear();
    cube.zeroCube();
    SINVARIANT(cube.size() == 0);

    return 0;
}

#if 0

#!/usr/bin/perl
my %base = ('0,t,a' => [1,1], '1,f,b' => [1,2], '2,f,a' => [1,3]);
my %zbase;
my %cube;
# stupid implementation, but different than the one in StatsCube, 
# so better for regression testing.
my $default_count = 0;
foreach my $a (qw/0 1 2/) {
    foreach my $b (qw/t f/) {
	foreach my $c (qw/a b/) {
	    $zbase{"$a,$b,$c"} = $base{"$a,$b,$c"} || [$default_count,0];
	}
    }
}

foreach my $a (qw/0 1 2/) {
    foreach my $b (qw/t f/) {
	foreach my $c (qw/a b/) {
	    rollup($a,$b,$c);
	}
    }
}

my $c = 0;
foreach my $key (sort keys %cube) {
    my $nkey = $key;
    $nkey =~ s/\./\*/go;
    print qq[ { "$nkey", $cube{$key}->[0], $cube{$key}->[1] },];
    ++$c; 
    if ($c > 4) {
	$c = 0;
	print "\n";
    }
}
print "\n";

sub rollup {
    my ($a,$b,$c) = @_;

    rollOne($a,$b,$c);
    rollOne($a,$b,'.');
    rollOne($a,'.',$c);
    rollOne($a,'.','.');
    rollOne('.',$b,$c);
    rollOne('.',$b,'.');
    rollOne('.','.',$c);
    rollOne('.','.','.');
}

sub rollOne {
    my $key = join(",", @_);

    return if defined $cube{$key};
    @keys = grep(/$key/, keys %zbase);
    my $count = 0;
    my $sum = 0;
    map { $count += $zbase{$_}->[0]; $sum += $zbase{$_}->[1] } @keys;
    $cube{$key} = [ $count, $sum ];
}

#endif
