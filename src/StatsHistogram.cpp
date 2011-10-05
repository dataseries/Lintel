/* -*-C++-*-
   (c) Copyright 1993-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Statistics/Histogram source code
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/StatsHistogram.hpp>
#include <Lintel/Double.hpp>

//////////////////////////////////////////////////////////////////////////////
// Basic StatsHistogram base class
//////////////////////////////////////////////////////////////////////////////

StatsHistogram::StatsHistogram() : Stats()
{ }

StatsHistogram::~StatsHistogram() { }

void StatsHistogram::add(const Stats &) {
    FATAL_ERROR("unimplemented");
}

void StatsHistogram::reset() {
    Stats::reset();
}


//////////////////////////////////////////////////////////////////////////////
// StatsHistogramUniform
//////////////////////////////////////////////////////////////////////////////

StatsHistogramUniform::StatsHistogramUniform(const unsigned bins_in,
			       const double   low_in,
			       const double   high_in, 
			       const bool     scalable_in,
			       const bool     growable_in)
      :	StatsHistogram(), 		// base statistic-gathering code
	num_bins(2*((bins_in+1)/2)), // round up to an even number
	bin_low(low_in),
	bin_high(high_in),
	bins(NULL),
	is_scalable(scalable_in),
	is_growable(growable_in),
	num_rescales(0),
	num_grows(0)
{
    INVARIANT(high_in >= low_in, "StatsHistogramUniform: high < low");
    INVARIANT(bins_in > 0, "StatsHistogramUniform: need at least one bin");
    INVARIANT(!(is_growable && is_scalable),
	      "StatsHistogramUniform: growable and stackable not allowed");

    bins = new unsigned long[num_bins];
    SINVARIANT(bins != NULL);
    reset();
}


StatsHistogramUniform::~StatsHistogramUniform() {
    delete[] bins;
    bins = NULL;
}


void StatsHistogramUniform::reset() {
    StatsHistogram::reset();	// This counts the resets for us
    for (unsigned bin=0; bin < num_bins; bin++)
	bins[bin] = 0;
    bin_width = ((double)(bin_high - bin_low))/(double)num_bins;
    num_rescales = 0;
}



unsigned long StatsHistogramUniform::operator[](unsigned index) const {
    SINVARIANT(checkInvariants());
    SINVARIANT(index <= num_bins);
    return bins[index];
}

unsigned StatsHistogramUniform::sampleBin(const double value) const {
    unsigned bin;		// Target bin for this number
    if (value <= bin_low)
	bin = 0;
    else if (value >= bin_high)
	bin = num_bins - 1;
    else
	bin = (unsigned)(floor((value-bin_low)/bin_width));

    return bin;
}

void StatsHistogramUniform::add(const double value) {
    Stats::add(value);	// Keep track of means, etc

    // Grow the number of bins?
    if (is_growable  &&  (value < bin_low || value >= bin_high)) {
	num_rescales++;		// About to do a reset, so count it
	
	if (value >= bin_high) { 
	    INVARIANT(value < std::numeric_limits<double>::max()/2, 
		      boost::format("StatsHistogramUniform: unreasonably"
				    "large upper value: %g") % value);
	    unsigned new_num_bins // round to an even number
		= 2 * ((unsigned)floor((value - bin_low) / bin_width) + 1)/2;
	    if (new_num_bins < num_bins * 2)
		new_num_bins = num_bins * 2; // at least double the bin count
	    unsigned long *new_bins = new unsigned long[new_num_bins];
	    unsigned long *old_bins = bins;

	    // zero the new bins, and then copy the old values over
	    memset(&new_bins[0], 0,        new_num_bins*sizeof(unsigned long));
	    memcpy(&new_bins[0], &old_bins[0], num_bins*sizeof(unsigned long));
	    //	    delete old_bins; // suppressed for now ...
	    bins = new_bins;
	    num_bins = new_num_bins;
	    bin_high = bin_low + new_num_bins * bin_width;
	} else {
	    INVARIANT(-value < std::numeric_limits<double>::max()/2,
		      boost::format("unreasonably large lower value: %lg")
		      % value);
	    unsigned new_num_bins // round to an even number
		= 2 * ((unsigned)floor((bin_high - value) / bin_width) + 1)/2;
	    if (new_num_bins < num_bins * 2)
		new_num_bins = num_bins * 2; // at least double the bin count
	    unsigned long *new_bins = new unsigned long[new_num_bins];
	    unsigned long *old_bins = bins;

	    // zero the new bins, and then copy the old values over
	    memset(new_bins, 0, sizeof(long) * new_num_bins);
	    memcpy(&new_bins[new_num_bins - num_bins],
		   &old_bins[0], num_bins*sizeof(unsigned long));
	    // delete old_bins;
	    bins = new_bins;
	    num_bins = new_num_bins;
	    bin_low = bin_high - new_num_bins * bin_width;
	}
    }

    if (is_scalable  &&  (value < bin_low || value >= bin_high)) {
	num_rescales++;		// About to do a reset, so count it

	while (value < bin_low) {	// Add bins to lower end of range
	    int i,j;
	    bin_low -= bin_high - bin_low;
	    for (i = num_bins-1, j = num_bins-2;
		 0 <= j;
		 i--, j-=2)
	    {
		bins[i] = bins[j]+bins[j+1];
	    }
	    while (i>=0) bins[i--] = 0;
	    bin_width += bin_width;
	}
	while (value >= bin_high) {	// Add bins to upper end of range
	    unsigned i,j;
	    bin_high += bin_high - bin_low;
	    for (i=0, j=0; i<(num_bins/2); i++, j+=2) {
		bins[i] = bins[j]+bins[j+1];
	    }
	    while (i < num_bins) bins[i++] = 0;
	    bin_width += bin_width;
	}
    } 

    unsigned bin = sampleBin(value);
    bins[bin]++;		// increment counts in bins
}

void StatsHistogramUniform::add(const double index_value, const double data_value) {
    INVARIANT(index_value == data_value,
	      "StatsHistogramUniform: attempted to add non-index value.");
    this->add(index_value);
}


// The mode is the center of the most popular bin

double StatsHistogramUniform::mode() const {
    SINVARIANT(checkInvariants());
    unsigned long max = bins[0];
    unsigned long maxpos = 0;
    for (unsigned long i=1; i<num_bins; i++) {
	if (max <= bins[i]) {
	    maxpos = i;
	    max = bins[i];
	}
    }
    return double(bin_low + (maxpos+0.5)*bin_width);
}


double StatsHistogramUniform::percentile(double p) const {
    SINVARIANT(checkInvariants());
    INVARIANT(0.0 <= p  && p <= 1.0, "argument is outside range");

    double targetnumber = p * count();
    unsigned long lowend = 0;
    unsigned long highend = bins[0];
    unsigned long bin = 0;
    while (Double::gt(targetnumber,highend)) {
	lowend = highend;
	highend += bins[++bin];
    }

    // extrapolate along the line between the lowend and highend to 
    // get the estimated value.  Note, that for any cell, the real
    // max value is i*bin_width + bin_width - small delta
    //
    if (highend == lowend) {
	// we hit a point where highend == lowend == targetnumber
	SINVARIANT(bin == 0);  // the only place where this can occur?
	return 0; 
    }
    return (bin_low + bin_width*(bin+(targetnumber-lowend)/(highend-lowend)));
}



//----------------------------------------------------------------
// Printing functions
//----------------------------------------------------------------


// Emit a string that summarizes the histogram data.

std::string StatsHistogramUniform::debugString() const {
    SINVARIANT(checkInvariants());

    std::string stat = Stats::debugString();
    if (count() == 0) {		// We have nothing more to add
	return stat;
    }

    return stat + boost::str(boost::format(" bins %u width %G range %G - %G rescales %u mode %G")
			     % num_bins % bin_width % bin_low % bin_high 
			     % num_rescales % double(mode()));
};


void StatsHistogramUniform::printRome(int depth, std::ostream &out) const {
    SINVARIANT(checkInvariants());

    std::string spaces;
    for(int i = 0; i < depth; i++) {
	spaces += " ";
    }

    Stats::printRome(depth,out);
    out << spaces << "{ histogram {\n";
    out << spaces << "  { type uniform }\n";
    out << spaces << "  { low " << low() << " }\n";
    out << spaces << "  { high " << high() << " }\n";
    out << spaces << "  { bins " << numBins() << " }\n";
    out << spaces << "  { binwidth " << binWidth() << " }\n";
    if (numRescales() > 0) {
	out << spaces << "  { rescales " << numRescales() << " }\n";
    }
    if (numGrows() > 0) {
	out << spaces << "  { grows " << numGrows() << " }\n";
    }
    out << spaces <<             "  { binCounts (";
    for(unsigned int j = 0; j < numBins(); j++) {
	if ((j % 10) == 0 && j > 0) {
	    out << "\n" << spaces << "               ";
	} else if (j > 0) {
	    out << " ";
	}
	out << (*this)[j];
    }
    out << " ) } } }\n"; 
}

void StatsHistogram::printTabular(int depth, std::ostream &out) const {
    SINVARIANT(checkInvariants());

    std::string spaces("");
    /*
      for(int i = 0; i < depth; i++) {
      spaces += " ";
      }
    */
  
    out << spaces << "histogram begin\n";
    Stats::printTabular(depth,out);
    out << spaces << "type " << getType() << "\n";
    out << spaces << "low " << low() << "\n";
    out << spaces << "high " << high() << "\n";
    out << spaces << "bins " << numBins() << "\n";
    if (numRescales() > 0) {
	out << spaces << "rescales " << numRescales() << "\n";
    }
    if (numGrows() > 0) {
	out << spaces << "grows " << numGrows() << "\n";
    }
    out << spaces << "binCounts begin "
	<< "(low items sumitems %items(pdf) %total(cdf) binwidth)\n";
    long long int sum = 0;

    for(unsigned int j = 0; j < numBins(); j++) {
	sum += (*this)[j];
	out << binhigh(j) << " " << (*this)[j] << " " << sum << " " 
	    << ((*this)[j] / (double) count()) << " " 
	    << (sum / (double) count()) << " " 
	    << binWidth(j) << std::endl;
    } 

    out << "binCounts end\n";
    out << "histogram end\n";
}


void StatsHistogram::printHistogramRandomInput(std::ostream& out) const {
    out << numBins() << std::endl;
    for(unsigned int j = 0; j < numBins(); j++) {
	out << binhigh(j) << " " << (*this)[j] << std::endl;
    }
} 


Stats *StatsHistogramUniform::another_new() const {
    return new StatsHistogramUniform(num_bins, 
				     bin_low, 
				     bin_high,
				     is_scalable,
				     is_growable);
}




//////////////////////////////////////////////////////////////////////////////
// StatsHistogramLog: logarithmic histograms
//////////////////////////////////////////////////////////////////////////////

StatsHistogramLog::StatsHistogramLog(const unsigned bins_in,
				     const double   smallest_in,
				     const double   high_in,
				     bool	    scalable_in)
      :	StatsHistogram(), // base statistic-gathering code
	num_bins(2*((bins_in+1)/2)), // round up to an even number
	smallest_bin(smallest_in),
	bin_high(high_in),
        is_scalable(scalable_in),
        num_rescales(0),
	bins(NULL)
{
//   fprintf(stderr, "%s: bins_in %d low_in %f high_in %f\n",
// 	     name(), bins_in, low_in, high_in);
//   for (unsigned i=0; i<num_bins; i++) {
//       fprintf(stderr, "%s: bin %d -> width %f [%f .. %f)\n",
// 		 name(), i, binWidth(i), binlow(i), binhigh(i));
//   }
//   for (double s=low_in; s<=high_in; s+= (high_in-low_in)/(bins_in*3)) {
//       fprintf(stderr, "%s: sample %f -> bin %f\n", name(), s, sampleBin(s));
//   }
  
    INVARIANT(bins_in > 0, "StatsHistogramLog: need at least one bin");
    INVARIANT(smallest_bin > 0.0, 
	      "StatsHistogramLog: low-bound must be > 0.0");
    INVARIANT(smallest_bin <= bin_high, 
	      "StatsHistogramLog: higher-bound must be larger than lower");

    bin_scaling = double(num_bins) / log(bin_high/smallest_bin);

    bins = new unsigned long[num_bins];
    reset();
}


StatsHistogramLog::~StatsHistogramLog() {
    delete[] bins;
    bins = NULL;
}


void StatsHistogramLog::reset() {
    StatsHistogram::reset();	// This counts the resets for us
    for (unsigned bin=0; bin < num_bins; bin++) {
	bins[bin] = 0;
    }
    num_rescales = 0;
}



unsigned long StatsHistogramLog::operator[](unsigned index) const {
    SINVARIANT(checkInvariants());
    SINVARIANT(index <= num_bins);
    return bins[index];
}


unsigned StatsHistogramLog::sampleBin(double x) const {
    if (x <= smallest_bin)
	return 0;
    double d_bin = bin_scaling * log(x / smallest_bin);
    unsigned bin = (unsigned)floor(d_bin);
    if (bin >= num_bins)
	return num_bins - 1;
    else
	return bin;
};

// Given a fractional bin-index, return the value that would be "stored" there.
//
double StatsHistogramLog::binOffset(double index) const {
    double ret = smallest_bin * exp(index/bin_scaling);
    return ret;
};



void StatsHistogramLog::add(const double value) {
    INVARIANT(value > 0.0, 
	      "values must be greater than 0 for log histogram");
    Stats::add(value);		// Keep track of means, etc

    if (is_scalable && (value < smallest_bin || value >= bin_high)) {
	num_rescales++;		// About to do a rescale, so count it

	while (value < smallest_bin) {	// Add bins to lower end of range
	    bin_scaling /= 2.0;
	    smallest_bin = bin_high / exp(num_bins / bin_scaling);
	    int i, j;
	    for (i = num_bins-1, j = num_bins-2; 0 <= j; i--, j-=2) {
		bins[i] = bins[j]+bins[j+1];
	    }
	    while (i>=0) bins[i--] = 0;
	}
	while (value >= bin_high) {	// Add bins to upper end of range
	    bin_scaling /= 2.0;
	    bin_high = smallest_bin * exp(num_bins / bin_scaling);
	    unsigned i, j;
	    for (i=0, j=0; i<(num_bins/2); i++, j+=2) {
		bins[i] = bins[j]+bins[j+1];
	    }
	    while (i < num_bins) bins[i++] = 0;
	}
    }

    unsigned bin = sampleBin(value);
    bins[bin]++;
}

void StatsHistogramLog::add(const double index_value, const double data_value) {
    INVARIANT(index_value == data_value,
	      "StatsHistogramLog: attempted to add non-index value.");
    this->add(index_value);
}

// The mode is the center of the most popular bin
//
double StatsHistogramLog::mode() const {
    SINVARIANT(checkInvariants());
    unsigned long max = bins[0];
    unsigned long maxpos = 0;
    for (unsigned long i=1; i<num_bins; i++) {
	if (max <= bins[i]) {
	    maxpos = i;
	    max = bins[i];
	}
    }
    return bincenter(maxpos);
}


double StatsHistogramLog::percentile(double p) const {
    SINVARIANT(checkInvariants());
    INVARIANT(0.0 <= p && p <= 1.0, "argument is outside range");

    double targetnumber = p * count();
    unsigned long lowend = 0;
    unsigned long highend = bins[0];
    unsigned long bin = 0;
    while (!(targetnumber <= highend)) {
	lowend = highend;
	highend += bins[++bin];
    }

    // extrapolate along the line between the lowend and highend to 
    // get the estimated value.  Note, that for any cell, the real
    // max value is i*bin_width + bin_width - small delta
    //
    if (highend == lowend) {
	// we hit a point where highend == lowend == targetnumber
	SINVARIANT(bin == 0);  // the only place where this can occur?
	return 0; 
    }
    return (binOffset(bin+(targetnumber-lowend)/(highend-lowend)));
}



//----------------------------------------------------------------
// Printing functions
//----------------------------------------------------------------


// Emit a string that summarizes the histogram data.
//
std::string StatsHistogramLog::debugString() const {
    SINVARIANT(checkInvariants());

    std::string stat = Stats::debugString();
    if (count() == 0) {		// We have nothing more to add
	return stat;
    }

    return stat + boost::str(boost::format(" bins %u smallest %G high %G mode %G")
			     % num_bins % smallest_bin % bin_high 
			     % double(mode()));
}

void StatsHistogramLog::printRome(int depth, std::ostream &out)  const{
    SINVARIANT(checkInvariants());

    std::string spaces;
    for(int i = 0; i < depth; i++) {
	spaces += " ";
    }

    Stats::printRome(depth,out);
    out << spaces << "{ histogram {\n";
    out << spaces << "  { type log }\n";
    out << spaces << "  { low " << low() << " }\n";
    //    out << spaces << "  { small " << smallest_bin << " }\n";
    out << spaces << "  { high " << high() << " }\n";
    out << spaces << "  { bins " << numBins() << " }\n";
    out << spaces <<             "  { binCounts (";
    for(unsigned int j = 0; j < numBins(); j++) {
	if ((j % 10) == 0 && j > 0) {
	    out << "\n" << spaces << "               ";
	} 
	else if (j > 0) {
	    out << " ";
	}
	out << (*this)[j];
    }
    out << " ) } } }\n";
}

Stats * StatsHistogramLog::another_new() const {
    return new StatsHistogramLog(num_bins, 
				 smallest_bin,
				 bin_high,
				 is_scalable);
}






//////////////////////////////////////////////////////////////////////////////
// Uniform histograms with accumulation
//////////////////////////////////////////////////////////////////////////////

StatsHistogramUniformAccum::StatsHistogramUniformAccum(const unsigned bins_in,
					       const double   low_in,
					       const double   high_in,
					       const bool     scalable_in,
					       const bool     growable_in)
  :	StatsHistogramUniform(bins_in, low_in, high_in, 
			      scalable_in, growable_in),
	val_bins(NULL)
{
  val_bins = new double[bins_in];
  SINVARIANT(val_bins != NULL);
  reset();
}

StatsHistogramUniformAccum::~StatsHistogramUniformAccum() {
    delete[] val_bins;
    val_bins = NULL;
}

void StatsHistogramUniformAccum::reset() {
    StatsHistogramUniform::reset();	// This counts the resets for us
    for (unsigned bin=0; bin < num_bins; bin++)
	val_bins[bin] = 0.0;
}

void StatsHistogramUniformAccum::add(const double index_value, 
                                     const double data_value) {
    Stats::add(data_value);		// Keep track of means, etc

    if (is_scalable  &&  (index_value < bin_low || index_value >= bin_high)) {
	num_rescales++;		// About to do a reset, so count it

	while (index_value < bin_low) {	// Add bins to lower end of range
	    int i,j;
	    bin_low -= bin_high - bin_low;
	    for (i = num_bins-1, j = num_bins-2;
		 0 <= j;
		 i--, j-=2)
	    {
		bins[i] = bins[j]+bins[j+1];
		val_bins[i] = val_bins[j]+val_bins[j+1];
	    }
	    while (i>=0) {
	      bins[i] = 0;
	      val_bins[i] = 0;
	      i--;
	    }
	    bin_width += bin_width;
	}
	while (index_value >= bin_high) {	// Add bins to upper end of range
	    unsigned i,j;
	    bin_high += bin_high - bin_low;
	    for (i=0, j=0; i<(num_bins/2); i++, j+=2) {
		bins[i] = bins[j]+bins[j+1];
		val_bins[i] = val_bins[j]+val_bins[j+1];
	    }
	    while (i < num_bins) {
	      bins[i] = 0;
	      val_bins[i] = 0;
	      i++;
	    }
	    bin_width += bin_width;
	}
    } 

    unsigned bin = sampleBin(index_value);
    bins[bin]++;		// and counts in bins
    val_bins[bin] += data_value;		// add value 
}

void StatsHistogramUniformAccum::add(const double value) {
    add(value, value);
}


double StatsHistogramUniformAccum::value(unsigned index) const {
    SINVARIANT(checkInvariants());
    SINVARIANT(index <= num_bins);
    return val_bins[index];
}


//----------------------------------------------------------------
// Printing functions
//----------------------------------------------------------------


void StatsHistogramUniformAccum::printRome(int depth, std::ostream &out)  const {
    SINVARIANT(checkInvariants());

    std::string spaces;
    for(int i = 0; i < depth; i++) {
	spaces += " ";
    }

    Stats::printRome(depth,out);
    out << spaces << "{ histogram {\n";
    out << spaces << "  { type uniformAccum }\n";
    out << spaces << "  { low " << low() << " }\n";
    out << spaces << "  { high " << high() << " }\n";
    out << spaces << "  { bins " << numBins() << " }\n";
    if (numRescales() > 0) {
	out << spaces << "  { rescales " << numRescales() << " }\n";
    }	
    if (numGrows() > 0) {
	out << spaces << "  { grows " << numGrows() << " }\n";
    }
    out << spaces <<             "  { binCounts (";
    unsigned int j;
    for(j = 0; j < numBins(); j++) {
	if ((j % 10) == 0 && j > 0) {
	    out << "\n" << spaces << "               ";
	} 
	else if (j > 0) {
	    out << " ";
	}
	out << (*this)[j];
    }
    out << ")}\n";
    out << spaces <<             "  { binMeans (";
    for(j = 0; j < numBins(); j++) {
	if ((j % 5) == 0 && j > 0) {
	    out << "\n" << spaces << "              ";
	} 
	else if (j > 0) {
	    out << " ";
	}
	out << this->value(j)/ (double)((*this)[j]);
    }
    out << " ) } } }\n";
}

Stats * StatsHistogramUniformAccum::another_new() const {
    return new StatsHistogramUniformAccum(num_bins, 
				      bin_low,
				      bin_high,
				      is_scalable,
				      is_growable);
}






//////////////////////////////////////////////////////////////////////////////
// Logarithmic histograms with accumulation
//////////////////////////////////////////////////////////////////////////////

StatsHistogramLogAccum::StatsHistogramLogAccum(const unsigned bins_in,
					       const double   smallest_in,
					       const double   high_in)
  :	StatsHistogramLog(bins_in, smallest_in, high_in),
	val_bins(NULL)
{
  val_bins = new double[num_bins];
  SINVARIANT(val_bins != NULL);
  reset();
}

StatsHistogramLogAccum::~StatsHistogramLogAccum() {
    delete[] bins;
    bins = NULL;
}

void StatsHistogramLogAccum::reset() {
    StatsHistogramLog::reset();	// This counts the resets for us
    for (unsigned bin=0; bin < num_bins; bin++)
	val_bins[bin] = 0.0;
}

void StatsHistogramLogAccum::add(const double value) {
    Stats::add(value);		// Keep track of means, etc
    
    unsigned bin = sampleBin(value);
    bins[bin]++;		// and counts in bins
    val_bins[bin] += value;
}


void StatsHistogramLogAccum::add(const double index_value, const double data_value) {
    Stats::add(data_value);		// Keep track of means, etc
    
    unsigned bin = sampleBin(index_value);
    bins[bin]++;		// and counts in bins
    val_bins[bin] += data_value;
}

double StatsHistogramLogAccum::value(unsigned index) const {
    SINVARIANT(checkInvariants());
    SINVARIANT(index <= num_bins);
    return val_bins[index];
}

//----------------------------------------------------------------
// Printing functions
//----------------------------------------------------------------

void StatsHistogramLogAccum::printRome(int depth, std::ostream &out) const {
    SINVARIANT(checkInvariants());

    std::string spaces;
    for(int i = 0; i < depth; i++) {
	spaces += " ";
    }

    Stats::printRome(depth,out);
    out << spaces << "{ histogram {\n";
    out << spaces << "  { type logAccum }\n";
    out << spaces << "  { low " << low() << " }\n";
    out << spaces << "  { high " << high() << " }\n";
    out << spaces << "  { bins " << numBins() << " }\n";
    out << spaces <<             "  { binCounts (";
    unsigned int j;
    for(j = 0; j < numBins(); j++) {
	if ((j % 10) == 0 && j > 0) {
	    out << "\n" << spaces << "               ";
	} 
	else if (j > 0) {
	    out << " ";
	}
	out << (*this)[j];
    }
    out << ")}\n";
    out << spaces <<             "  { binMeans (";
    for(j = 0; j < numBins(); j++) {
	if ((j % 5) == 0 && j > 0) {
	    out << "\n" << spaces << "              ";
	} 
	else if (j > 0) {
	    out << " ";
	}
	out << this->value(j)/ (double)((*this)[j]);
    }
    out << " ) } } } \n";
}

Stats *StatsHistogramLogAccum::another_new() const {
    return new StatsHistogramLogAccum(num_bins, 
				      smallest_bin,
				      bin_high);
}

StatsHistogramGroup::StatsHistogramGroup(const StatsHistogram::HistMode mode, 
					 const int buckets, 
					 const std::vector<double> &_ranges)
  : Stats(), ranges(_ranges), low(), high()
{
    INVARIANT(ranges.size() >= 2, "StatsHistogramGroup: must have at"
	      "least two values in ranges vector.");
    histograms.resize(ranges.size()-1);
    for(unsigned int i=0;i<histograms.size();i++) {
	switch(mode) 
	    {
	    case StatsHistogram::Uniform:
		histograms[i] = new StatsHistogramUniform(buckets,ranges[i],ranges[i+1]);
		break;
	    case StatsHistogram::Log:
		histograms[i] = new StatsHistogramLog(buckets,ranges[i],ranges[i+1]);
		break;
	    case StatsHistogram::UniformAccum:
		histograms[i] = new StatsHistogramUniformAccum(buckets,ranges[i],ranges[i+1]);
		break;
	    case StatsHistogram::LogAccum:
		histograms[i] = new StatsHistogramLogAccum(buckets,ranges[i],ranges[i+1]);
		break;
	    default:
		FATAL_ERROR("I didn't get here.");
	    }
    }
}

StatsHistogramGroup::StatsHistogramGroup(const std::vector<StatsHistogram::HistMode> &_modes, 
					 const std::vector<int> &buckets, 
					 const std::vector<double> &_ranges)
  : Stats(), modes(_modes), ranges(_ranges), low(), high()
{
    INVARIANT(ranges.size() >= 2, "StatsHistogramGroup: must have at least"
	      " two values in ranges vector.");
    INVARIANT(modes.size() == (ranges.size() - 1), "StatsHistogramGroup:"
	      " must supply one mode per range in mode vector.");
		 
    INVARIANT(buckets.size() == (ranges.size() - 1), "StatsHistogramGroup: "
	      "must supply one bucket value per range in buckets vector.");
    histograms.resize(ranges.size()-1);
    for(unsigned int i=0;i<histograms.size();i++) {
	switch(modes[i]) 
	    {
	    case StatsHistogram::Uniform:
		histograms[i] = new StatsHistogramUniform(buckets[i],ranges[i],ranges[i+1]);
		break;
	    case StatsHistogram::Log:
		histograms[i] = new StatsHistogramLog(buckets[i],ranges[i],ranges[i+1]);
		break;
	    case StatsHistogram::UniformAccum:
		histograms[i] = new StatsHistogramUniformAccum(buckets[i],ranges[i],ranges[i+1]);
		break;
	    case StatsHistogram::LogAccum:
		histograms[i] = new StatsHistogramLogAccum(buckets[i],ranges[i],ranges[i+1]);
		break;
	    default:
		FATAL_ERROR("I didn't get here.");
	    }
    }
}

StatsHistogramGroup::~StatsHistogramGroup() {
  for(unsigned int i=0;i<histograms.size();i++) {
    delete histograms[i];
    histograms[i] = NULL;
  }
}

void StatsHistogramGroup::reset() {
  Stats::reset();
  low.reset();
  high.reset();
  for(unsigned int i=0;i<histograms.size();i++) {
    histograms[i]->reset();
  }
}

void StatsHistogramGroup::add(const double value) {
  Stats::add(value);
  if (value < ranges[0]) {
    low.add(value);
    return;
  }
  for(unsigned int i=1;i<ranges.size();i++) {
    if (value < ranges[i]) {
      histograms[i-1]->add(value);
      return;
    }
  }
  high.add(value);
  return;
}

void StatsHistogramGroup::add(const double index_value, const double data_value) {
  Stats::add(data_value);
  if (index_value < ranges[0]) {
    low.add(data_value);
    return;
  }
  for(unsigned int i=1;i<ranges.size();i++) {
    if (index_value < ranges[i]) {
      histograms[i-1]->add(index_value, data_value);
      return;
    }
  }
  high.add(data_value);
  return;
}

void StatsHistogramGroup::add(const Stats &) {
    FATAL_ERROR("unimplemented");
}
    
void StatsHistogramGroup::printRome(int depth, std::ostream &out)  const {
  std::string spaces;
  for(int i = 0; i < depth; i++) {
    spaces += " ";
  }

  Stats::printRome(depth,out);
  out << spaces << "{ histogramGroup {\n";
  out << spaces << "  { low {\n";
  low.printRome(depth+4,out);
  out << spaces << "  }}\n";
  out << spaces << "  { high {\n";
  high.printRome(depth+4,out);
  out << spaces << "  }}\n";
  out << spaces <<   "  { histograms (\n";
  for(unsigned int j = 0;j<histograms.size();j++) {
    out << spaces << "    {\n";
    histograms[j]->printRome(depth+6,out);
    out << spaces << "    }\n";
  }
  out << spaces << " ) } } }\n";
}
