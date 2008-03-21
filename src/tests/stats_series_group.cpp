/* -*-C++-*-
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    regression test
*/

#include <StatsSeriesGroup.H>
#include <MersenneTwisterRandom.H>
#include "streamcompat.H"

int
main(int argc, char *argv[])
{
    MersenneTwisterRandom rand(1972); // an author related year

    StatsSeriesGroup test("test",100);

    for(double ts=0;ts<1000;ts+=0.1) {
	test.addTimeSeq(rand.randDouble(),ts);
    }
    test.addTimeSeq(1, 1499);
    test.addTimeSeq(5, 1500);
    test.addTimeSeq(7, 1776);
    std::cout << "stats test {\n";
    test.printRome(2, std::cout);
    std::cout << "};\n\n";
}
