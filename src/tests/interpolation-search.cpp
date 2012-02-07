/*
   (c) Copyright 2012, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Interpolation search template
*/

#include <vector>
#include <iostream>

#include <boost/format.hpp>

#include <Lintel/DebugFlag.hpp>
#include <Lintel/InterpolationSearch.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>

using namespace std;
using lintel::interpolationLowerBound;
using boost::format;

// static global so that the fact that std::lower_bound copies the comparator doesn't cause
// any problems.

size_t eo_count, compare_count;

template<typename ValueT> struct EstimateOffsetCount {
    size_t 
    operator()(const ValueT &v, const ValueT &first, const ValueT &last, size_t last_pos) const {
        ++eo_count;
        ValueT value_range = last - first;
        double relative_pos = static_cast<double>(v - first) / value_range; // [0,1]
        return relative_pos * last_pos;
    }

};

template<typename ValueT> struct CompareCount {
    bool operator()(const ValueT &a, const ValueT &b) const {
        ++compare_count;
        return a < b;
    }
};

template<typename ValueT, typename Iterator> Iterator
countingILB(Iterator begin, Iterator end, const ValueT &v) {
    EstimateOffsetCount<ValueT> eo;
    return lintel::detail::interpolationLowerBound<2,4,256>(begin, end, v, eo, 
                                                            CompareCount<ValueT>());
}

void testSimple() {
    vector<int> array;

    for(int i=0; i < 1000; ++i) {
        array.push_back(i);
    }

    SINVARIANT(interpolationLowerBound<int>(array.begin(), array.end(), -1) == array.begin());
    SINVARIANT(interpolationLowerBound(array.begin(), array.end(), 1000) == array.end());
    for(int i=0; i < 1000; ++i) {
        vector<int>::iterator iter = interpolationLowerBound(array.begin(), array.end(), i);
        SINVARIANT(iter >= array.begin() && iter < array.end());
        SINVARIANT(*iter == i);
    }

    // Now with counting
    SINVARIANT(countingILB<int>(array.begin(), array.end(), -1) == array.begin()); // 1 comparison
    SINVARIANT(countingILB(array.begin(), array.end(), 1000) == array.end()); // 2 comparisons
    for(int i=0; i < 1000; ++i) {
        // 1 comparison for 0 (same as -1), 1 eo, 4 comparisons for rest
        vector<int>::iterator iter = countingILB(array.begin(), array.end(), i);
        SINVARIANT(iter >= array.begin() && iter < array.end());
        SINVARIANT(*iter == i);
        cout << format("%d offset estimations, %d comparisions\n") % eo_count % compare_count;
    }

    size_t expect_compare = 1 + 2 + 1 + 4 * 999;
    IF_LINTEL_DEBUG(expect_compare += 2 * 999); // debug checks cost another 2 compares/cycle
    SINVARIANT(eo_count == 999 && compare_count == expect_compare);

    cout << "test simple passed.\n";
}

void testRandom() {
    MersenneTwisterRandom rng;
    cout << format("test random seed=%d...") % rng.seed_used;

    static const size_t nvals = 256 * 1024;

    vector<int> array;

    for(size_t i=0; i < nvals; ++i) {
        // * 16 == some duplicates, mostly unique, efficient calculation
        array.push_back(rng.randInt(nvals*16)); 
    }
    sort(array.begin(), array.end());

    for(size_t i=0;i < nvals; ++i) {
        int v = rng.randInt(nvals*10);
        vector<int>::iterator ilb = interpolationLowerBound(array.begin(), array.end(), v);
        vector<int>::iterator lb = lower_bound(array.begin(), array.end(), v);
        INVARIANT(ilb == lb, format("for %d: %d != %d") % v % (ilb == array.end() ? -1 : *ilb)
                  % (lb == array.end() ? -1 : *lb));
    }

    cout << "passed.\n";
}

int main() {
    testSimple();
    testRandom();
    return 0;
}
