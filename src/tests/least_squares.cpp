/* -*-C++-*- */
/*
   (c) Copyright 2007, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    test 
*/

#include <iostream>

#include <Lintel/AssertBoost.H>
#include <Lintel/Double.H>
#include <Lintel/LeastSquares.hpp>

using namespace std;

int main()
{
    LeastSquares::WeightedData test;

    test.push_back(LeastSquares::WeightedPoint(1,1));
    test.push_back(LeastSquares::WeightedPoint(2,2));
    test.push_back(LeastSquares::WeightedPoint(3,3));
    test.push_back(LeastSquares::WeightedPoint(4,4));
    test.push_back(LeastSquares::WeightedPoint(5,5));
    test.push_back(LeastSquares::WeightedPoint(6,6));

    LeastSquares::Linear result = LeastSquares::fitLinearVertical(test);
    INVARIANT(result.intercept == 0 && result.slope == 1, "bad");
    
    test.clear();
    test.push_back(LeastSquares::WeightedPoint(24037007,-321.5));
    test.push_back(LeastSquares::WeightedPoint(27040842,-322));
    test.push_back(LeastSquares::WeightedPoint(28042496,-323));
    test.push_back(LeastSquares::WeightedPoint(29043391,-314));
    test.push_back(LeastSquares::WeightedPoint(31044518,-322));
    test.push_back(LeastSquares::WeightedPoint(39048310,-311));
    test.push_back(LeastSquares::WeightedPoint(62067122,-317.5));
    test.push_back(LeastSquares::WeightedPoint(116106946,-300));
    test.push_back(LeastSquares::WeightedPoint(117107339,-301.5));
    test.push_back(LeastSquares::WeightedPoint(118108081,-290));
    test.push_back(LeastSquares::WeightedPoint(119109345,-296));
    test.push_back(LeastSquares::WeightedPoint(120110256,-294.5));
    test.push_back(LeastSquares::WeightedPoint(122113491,-289));
    test.push_back(LeastSquares::WeightedPoint(125114789,-293.5));
    test.push_back(LeastSquares::WeightedPoint(126115896,-296.5));
    test.push_back(LeastSquares::WeightedPoint(127116738,-292.5));
    test.push_back(LeastSquares::WeightedPoint(128116805,-283));
    test.push_back(LeastSquares::WeightedPoint(129116889,-285));
    test.push_back(LeastSquares::WeightedPoint(130116954,-281));
    test.push_back(LeastSquares::WeightedPoint(131117031,-279));
    test.push_back(LeastSquares::WeightedPoint(132117104,-279));
    test.push_back(LeastSquares::WeightedPoint(133117182,-278));
    test.push_back(LeastSquares::WeightedPoint(134117254,-273));
    test.push_back(LeastSquares::WeightedPoint(135117331,-275.5));
    test.push_back(LeastSquares::WeightedPoint(136118414,-278.5));
    test.push_back(LeastSquares::WeightedPoint(137119481,-275));
    test.push_back(LeastSquares::WeightedPoint(138119556,-274.5));
    test.push_back(LeastSquares::WeightedPoint(139119633,-276));
    test.push_back(LeastSquares::WeightedPoint(140119706,-273.5));
    test.push_back(LeastSquares::WeightedPoint(141119784,-273));
    test.push_back(LeastSquares::WeightedPoint(142119857,-273));
    test.push_back(LeastSquares::WeightedPoint(143119954,-276));
    test.push_back(LeastSquares::WeightedPoint(144120009,-268));
    test.push_back(LeastSquares::WeightedPoint(145120085,-269));
    test.push_back(LeastSquares::WeightedPoint(146120159,-269.5));
    test.push_back(LeastSquares::WeightedPoint(147120235,-271));
    test.push_back(LeastSquares::WeightedPoint(148120311,-267.5));
    test.push_back(LeastSquares::WeightedPoint(149120387,-269));
    test.push_back(LeastSquares::WeightedPoint(150120469,-269.5));

    result = LeastSquares::fitLinearVertical(test);
    // Results calculated using perl Statistics::OLS module
    INVARIANT(Double::eq(result.intercept,-334.874623780947) && 
	      Double::eq(result.slope, 4.10303516778801e-07), "bad");

    cout << "Passed least_squares test\n";
    return 0;
}
