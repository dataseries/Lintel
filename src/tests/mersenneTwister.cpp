/* -*-C++-*-
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Regression test for the MersenneTwisterRandom class
*/


#include <iostream>
#include <boost/format.hpp>

using namespace std;

#include <Lintel/MersenneTwisterRandom.hpp>

void shuffleShow() {
    vector<int> distr;
    vector<int> base;
    distr.resize(10);
    base.resize(distr.size());
    for(uint32_t i = 0; i < base.size(); ++i) {
	base[i] = i;
    }

    MersenneTwisterRandom rng;

    vector<int> work;
    work.resize(base.size());
    for(uint32_t i = 0; i < 100000; ++i) {
	work = base;
	MT_random_shuffle(work.begin(), work.end(), rng);
	distr[work[0]] += 1;
    }

    for(uint32_t i = 0; i < distr.size(); ++i) {
	cout << boost::format("%d -> %d\n") % i % distr[i];
    }
}

int main() {
    MersenneTwisterRandom::selfTest();

    //    shuffleShow();
    return 0;
}

