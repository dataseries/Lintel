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

#include <Lintel/MarsagliaRandom.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>
#include <Lintel/Clock.hpp>

typedef Clock::Tfrac Tfrac;

using namespace lintel;

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

template <class R> void speedTest(R rng) {
        int64_t res = 0;
        int64_t reps = 1;
        Tfrac start, stop;
        Stats plain, modulus, dbl, boole, boolefast;

        do {
            reps *= 2;
            start = Clock::todTfrac();
            for(int64_t i = 0; i<reps; ++i) {
                res += rng.randInt();
            }
            stop = Clock::todTfrac();
        } while(Clock::TfracToDouble(stop-start) < 1.0);

        plain.add(Clock::TfracToDouble(stop-start) / reps * 1000000000.0);  // In nanos
        
        do {
            start = Clock::todTfrac();
            for(int64_t i = 0; i<reps; ++i) {
                res += rng.randInt();
            }
            stop = Clock::todTfrac();
            plain.add(Clock::TfracToDouble(stop-start) / reps * 1000000000.0);  // In nanos
        } while (plain.relconf95() > .05 || plain.count() < 10);
        cout << "For plain: " << plain.debugString() << endl;

        reps = 1;

        do {
            reps *= 2;
            start = Clock::todTfrac();
            for(int64_t i = 0; i<reps; ++i) {
                res += rng.randInt(1000);
            }
            stop = Clock::todTfrac();
        } while(Clock::TfracToDouble(stop-start) < 1.0);

        modulus.add(Clock::TfracToDouble(stop-start) / reps * 1000000000.0);  // In nanos
        
        do {
            start = Clock::todTfrac();
            for(int64_t i = 0; i<reps; ++i) {
                res += rng.randInt(1000);
            }
            stop = Clock::todTfrac();
            modulus.add(Clock::TfracToDouble(stop-start) / reps * 1000000000.0);  // In nanos
        } while (modulus.relconf95() > .05 || modulus.count() < 10);
        cout << "For modulus: " << modulus.debugString() << endl;


        reps = 1;
        double dres = res;
        do {
            reps *= 2;
            start = Clock::todTfrac();
            for(int64_t i = 0; i<reps; ++i) {
                dres += rng.randDouble();
            }
            stop = Clock::todTfrac();
        } while(Clock::TfracToDouble(stop-start) < 1.0);

        dbl.add(Clock::TfracToDouble(stop-start) / reps * 1000000000.0);  // In nanos
        
        do {
            start = Clock::todTfrac();
            for(int64_t i = 0; i<reps; ++i) {
                dres += rng.randDouble();
            }
            stop = Clock::todTfrac();
            dbl.add(Clock::TfracToDouble(stop-start) / reps * 1000000000.0);  // In nanos
        } while (dbl.relconf95() > .05 || dbl.count() < 10);
        cout << "For dbl: " << dbl.debugString() << endl;

        dres += res;


        reps = 1;

        do {
            reps *= 2;
            start = Clock::todTfrac();
            for(int64_t i = 0; i<reps; ++i) {
                res += rng.randBool();
            }
            stop = Clock::todTfrac();
        } while(Clock::TfracToDouble(stop-start) < 1.0);

        boole.add(Clock::TfracToDouble(stop-start) / reps * 1000000000.0);  // In nanos
        
        do {
            start = Clock::todTfrac();
            for(int64_t i = 0; i<reps; ++i) {
                res += rng.randBool();
            }
            stop = Clock::todTfrac();
            boole.add(Clock::TfracToDouble(stop-start) / reps * 1000000000.0);  // In nanos
        } while (boole.relconf95() > .05 || boole.count() < 10);
        cout << "For boole: " << boole.debugString() << endl;

        reps = 1;

        do {
            reps *= 2;
            start = Clock::todTfrac();
            for(int64_t i = 0; i<reps; ++i) {
                res += rng.randBoolFast();
            }
            stop = Clock::todTfrac();
        } while(Clock::TfracToDouble(stop-start) < 1.0);

        boolefast.add(Clock::TfracToDouble(stop-start) / reps * 1000000000.0);  // In nanos
        
        do {
            start = Clock::todTfrac();
            for(int64_t i = 0; i<reps; ++i) {
                res += rng.randBoolFast();
            }
            stop = Clock::todTfrac();
            boolefast.add(Clock::TfracToDouble(stop-start) / reps * 1000000000.0);  // In nanos
        } while (boolefast.relconf95() > .05 || boolefast.count() < 10);
        cout << "For boolefast: " << boolefast.debugString() << endl;


        cout << "Result was " << dres << endl;
}

int main() {
    MersenneTwisterRandom::selfTest();

    //    shuffleShow();


    if (0) { // Performnace test
        MersenneTwisterRandom mtr;
        MarsagliaRNGHuge mrhuge;
        MarsagliaRNGTiny mrtiny;
        MarsagliaRNGMicro mrmicro;

        speedTest<MarsagliaRNGMicro>(mrmicro);


        speedTest<MersenneTwisterRandom>(mtr);
        speedTest<MarsagliaRNGHuge>(mrhuge);
        speedTest<MarsagliaRNGTiny>(mrtiny);
    }


    return 0;
}

