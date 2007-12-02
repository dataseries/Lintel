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

    typedef std::vector<std::pair<double, double> > Data;

    static Linear fitLinearVertical(const Data &data);
    
    Linear fitLinearVertical() {
	return fitLinearVertical(data);
    }

    void add(double x, double y) {
	data.push_back(std::pair<double, double>(x,y));
    }

    /// space separated one pair/line
    void printText(std::ostream &to) const; 

    Data data;
};

inline std::ostream &operator<<(std::ostream &to, const LeastSquares &ls) {
    ls.printText(to);
    return to;
}

#endif
