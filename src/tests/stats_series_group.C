/* -*-C++-*-
*******************************************************************************
*
* File:         stats_series_group.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/tests/stats_series_group.C,v 1.2 2003/07/30 00:17:26 anderse Exp $
* Description:  regression test
* Author:       Eric Anderson
* Created:      Sun Dec 16 13:30:55 2001
* Modified:     Mon Jun 30 10:32:33 2003 (Alistair Veitch) aveitch@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
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
