/* -*-C++-*- */
/*
   (c) Copyright 2007, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Calculate an ordinary least squares over a dataset, along with a
    set of statistics. http://en.wikipedia.org/wiki/Least_squares
    http://mathworld.wolfram.com/LeastSquaresFitting.html
*/

#ifndef __LINTEL_LEASTSQUARES_H
#define __LINTEL_LEASTSQUARES_H

#include <ostream>
#include <utility>
#include <vector>

class LeastSquares {
public:
    struct Linear {
	double intercept, slope;
	// TODO: add in more statistics, e.g. R^2
    };
    struct WeightedPoint { 
	WeightedPoint(double _x, double _y, double _weight = 1.0) 
	    : x(_x), y(_y), weight(_weight) { }
	double x, y, weight; 
    };

    typedef std::vector<WeightedPoint> WeightedData;

    static Linear fitLinearVertical(const WeightedData &data);
    
    Linear fitLinearVertical() {
	return fitLinearVertical(data);
    }

    void add(double x, double y, double weight = 1.0) {
	data.push_back(WeightedPoint(x, y, weight));
    }

    /// space separated one pair/line
    void printText(std::ostream &to) const; 

    WeightedData data;
};

inline std::ostream &operator<<(std::ostream &to, const LeastSquares &ls) {
    ls.printText(to);
    return to;
}

#endif
