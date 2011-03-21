/* -*-C++-*- */
/*
   (c) Copyright 2007, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief header file for LeastSquares class

*/

#ifndef LINTEL_LEASTSQUARES_HPP
#define LINTEL_LEASTSQUARES_HPP

#include <ostream>
#include <utility>
#include <vector>

/// \brief LeastSquares matrix implementation.
///
/// Calculate an ordinary least squares over a dataset, along with a
/// set of statistics. http://en.wikipedia.org/wiki/Least_squares
/// http://mathworld.wolfram.com/LeastSquaresFitting.html
class LeastSquares {
public:
    /// \brief Return class for the least-squares fit
    struct Linear {
	double intercept, slope;

	double get(double x) const { return x * slope + intercept; }
	// TODO: add in more statistics, e.g. R^2
    };
    /// \brief Class for weighting the least-squares values.
    struct WeightedPoint { 
	WeightedPoint(double _x, double _y, double _weight = 1.0) 
	    : x(_x), y(_y), weight(_weight) { }
	double x, y, weight; 
    };

    typedef std::vector<WeightedPoint> WeightedData;

    
    Linear fitLinearVertical() {
	return fitLinearVertical(data);
    }

    // weights are relative, i.e., they need not add up to n; a
    // typical choice of weights is each weight equals the reciprocal
    // of the variance of the measurement
    void add(double x, double y, double weight = 1.0) {
	INVARIANT(weight > 0, "weight should be > 0");
	data.push_back(WeightedPoint(x, y, weight));
    }

    void clear() { 
	data.clear(); 
    }

    size_t size() {
	return data.size();
    }

    // Deprecated, remove 2008-06-01 or after
    size_t dataSize() { 
	return size();
    }
    /// space separated one pair/line
    void printText(std::ostream &to) const; 

private:
    static Linear fitLinearVertical(const WeightedData &data);

    WeightedData data;
};

inline std::ostream &operator<<(std::ostream &to, const LeastSquares &ls) {
    ls.printText(to);
    return to;
}

#endif
