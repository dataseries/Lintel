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
    LeastSquares test;

    // test simple uniform weight
    test.add(1,1);
    test.add(2,2);
    test.add(3,3);
    test.add(4,4);
    test.add(5,5);
    test.add(6,6);
    LeastSquares::Linear result = test.fitLinearVertical();
    INVARIANT(result.intercept == 0 && result.slope == 1, "bad");
    
    // test simple non-uniform weight
    test.clearData();
    test.add(-1,0, 0.25);
    test.add(0,1, 0.50);
    test.add(1,0, 0.25);
    result = test.fitLinearVertical();
    // compare against hand calculation
    INVARIANT(result.intercept == 0.5 && result.slope == 0, "bad");
    
    // test complex uniform weight
    test.clearData();
    test.add(24037007,-321.5);
    test.add(27040842,-322);
    test.add(28042496,-323);
    test.add(29043391,-314);
    test.add(31044518,-322);
    test.add(39048310,-311);
    test.add(62067122,-317.5);
    test.add(116106946,-300);
    test.add(117107339,-301.5);
    test.add(118108081,-290);
    test.add(119109345,-296);
    test.add(120110256,-294.5);
    test.add(122113491,-289);
    test.add(125114789,-293.5);
    test.add(126115896,-296.5);
    test.add(127116738,-292.5);
    test.add(128116805,-283);
    test.add(129116889,-285);
    test.add(130116954,-281);
    test.add(131117031,-279);
    test.add(132117104,-279);
    test.add(133117182,-278);
    test.add(134117254,-273);
    test.add(135117331,-275.5);
    test.add(136118414,-278.5);
    test.add(137119481,-275);
    test.add(138119556,-274.5);
    test.add(139119633,-276);
    test.add(140119706,-273.5);
    test.add(141119784,-273);
    test.add(142119857,-273);
    test.add(143119954,-276);
    test.add(144120009,-268);
    test.add(145120085,-269);
    test.add(146120159,-269.5);
    test.add(147120235,-271);
    test.add(148120311,-267.5);
    test.add(149120387,-269);
    test.add(150120469,-269.5);
    result = test.fitLinearVertical();
    // Results calculated using perl Statistics::OLS module
    INVARIANT(Double::eq(result.intercept,-334.874623780947) && 
	      Double::eq(result.slope, 4.10303516778801e-07), "bad");

    // test complex uniform weight
    // the following numbers are manually randomly typed in
    test.clearData();
    test.add(23, -3.5, 0.1);
    test.add(-2, 10, 0.05);
    test.add(2, 30.5, 0.3);
    test.add(100, 45.3, 1.2);
    test.add(26.4, 75.3, 2.0);
    test.add(4.3, 57.2, 1.0);
    test.add(24, 23.0, 3.0);
    test.add(53, -40.2, 0.1);
    test.add(134, 1034.2, 0.56);
    test.add(5234, -304.56, 1.23);
    test.add(1345, -201.34, 3.24);
    test.add(21, 1123, 7.96);
    test.add(1, 367.34, 3.22);
    test.add(1345, 560.345, 0.32);
    test.add(5678, 272.101, 7.11);
    test.add(7890, 909.1, 8.09);
    test.add(4245, 232.5, 10.01);
    test.add(2490, 934, 21.89);
    test.add(345, 10293.3, 0.005);
    test.add(34, 984, 7.36);
    result = test.fitLinearVertical();
    // Results calculated using perl Statistics::OLS module
    INVARIANT(Double::eq(result.intercept,  662.946833319873) && 
	      Double::eq(result.slope, -0.0124699082339257), "bad");

    cout << "Passed least_squares test\n";
    return 0;
}
