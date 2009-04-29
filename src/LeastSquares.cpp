/* -*-C++-*- */
/*
   (c) Copyright 2007, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    implementation
*/
#include <Lintel/AssertBoost.hpp>
#include <Lintel/LeastSquares.hpp>

#include <boost/format.hpp>

using namespace std;
using boost::format;

LeastSquares::Linear LeastSquares::fitLinearVertical(const WeightedData &data) {
    double sum_w = 0,  sum_wx = 0, sum_wy = 0, sum_wxx = 0, sum_wxy = 0;

    INVARIANT(data.size() > 1, "must have at least 2 points for linear fit");

    for (WeightedData::const_iterator i = data.begin(); i != data.end(); ++i) {
	sum_w += i->weight;
	sum_wx += i->weight * i->x;
	sum_wy += i->weight * i->y;
	sum_wxx += i->weight * i->x * i->x;
	sum_wxy += i->weight * i->x * i->y;
    }

    Linear ret;
    double denom = sum_w * sum_wxx - sum_wx * sum_wx;
    INVARIANT(denom != 0.0, "denominator is 0, aborting");
    ret.slope = (sum_w * sum_wxy - sum_wx * sum_wy) / denom;
    ret.intercept = (sum_wy - ret.slope * sum_wx) / sum_w;

    return ret;
}

void LeastSquares::printText(ostream &to) const {
    for(WeightedData::const_iterator i = data.begin(); i != data.end(); ++i) {
	to << boost::format("%24.18g %24.18g %24.18g\n") % i->x % i->y % i->weight;
    }
}
