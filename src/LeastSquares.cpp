/* -*-C++-*- */
/*
   (c) Copyright 2007, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    implementation
*/
#include <Lintel/AssertBoost.H>
#include <Lintel/LeastSquares.hpp>

#include <boost/format.hpp>

using namespace std;
using boost::format;

LeastSquares::Linear
LeastSquares::fitLinearVertical(const Data &data)
{
    double sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;

    INVARIANT(data.size() > 1, "must have at least 2 points for linear fit");
    for(Data::const_iterator i = data.begin(); i != data.end(); ++i) {
	sum_x += i->first;
	sum_y += i->second;
	sum_xx += i->first * i->first;
	sum_xy += i->first * i->second;
    }

    double n = data.size();
    Linear ret;
    ret.slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    ret.intercept = (sum_y - ret.slope * sum_x)/n;

    return ret;
}

void
LeastSquares::printText(ostream &to) const
{
    for(Data::const_iterator i = data.begin(); i != data.end(); ++i) {
	to << boost::format("%24.18g %24.18g\n") % i->first % i->second;
    }
}
