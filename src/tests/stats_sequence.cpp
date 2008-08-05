/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <iostream> 
#include <fstream>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/StatsSequence.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>

int
main()
{
    MersenneTwisterRandom rand(1776); // An excellent year.

    std::cout << "#>Rome-1-9\n";

    StatsSequence a(50);

    int i;

    for(i=0;i<50;i++) {
	a.add(rand.randDouble());
    }

    std::cout << "statsSequence a {\n";
    a.printRome(2,std::cout);
    std::cout <<"};\n\n";

    a.add(rand.randDouble());

    std::cout << "statsSequence a2 {\n";
    a.printRome(2,std::cout);
    std::cout <<"};\n\n";

    StatsSequence b1(10);
    StatsSequence b2(10);
    
    b1.setIntervalWidth(1);
    b2.setIntervalWidth(0.5);
    
    for(i=0;i<1280;i++) {
	double t1 = rand.randDouble();
	double t2 = rand.randDouble();

	b1.add(t1+t2);
	b2.add(t1);
	b2.add(t2);
    }

    for(i=0;i<10;i++) {
	INVARIANT(b1.get(i) == b2.get(i),
		  boost::format("Mismatch %.8f %.8f ??")
		  % b1.get(i) % b2.get(i));
    }
    std::cout << "statsSequence b1 {\n";
    b1.printRome(2,std::cout);
    std::cout <<"};\n\n";
    
    std::cout << "statsSequence b2 {\n";
    b2.printRome(2,std::cout);
    std::cout <<"};\n\n";

    exit(0);
}

