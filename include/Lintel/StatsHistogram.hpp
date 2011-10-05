/* -*-C++-*- */
/*
   (c) Copyright 1993-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief Header file for a histogram class.
*/

///////////////////////////////////////////////////////////////////////////////
//
// This package provides histogram gathering classes on variables.
// StatsHistograms can be used to approximate the mode and percentiles.
// 
// The histograms collect samples into bins, where the number and size of
// bins is determined when the histogram object is created.  Two
// variations are provided: one version rescales bins when values falling
// outside any bin are sampled, while the other lumps extreme values into
// the end bins.  The former are called `scaling' histograms while the
// latter are `fixed'.
// 
// When specifying the domain of a histogram, you specify the low and
// high values.  The domain is then all values low <= x < high.  For an
// integer histogram, where you are (say) interested in collecting values
// 0 through 11, specify 0 as the low and _12_ as the high point.
// 
// Note also that the histograms will round the number of bins up to an
// even number.  This may have unfortunate side-effects when working with
// integer histograms.  To be safe, you should always specify an even
// number of bins.  You can usually put an extra bin on the high or low
// end.
// 
// Note also that the `width' of bins is always a double, even in the
// integer histograms.  This is because width is calculated from the high
// and low points and the number of bins.  If you specify a funny
// combination of these, you might get (say) bins alternating with two
// and three bins.
// 
// There are those who believe that integer histograms probably aren't
// terribly useful.  They are probably right.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef LINTEL_STATSHISTOGRAM_HPP
#define LINTEL_STATSHISTOGRAM_HPP

#include <vector>
#include <math.h>
#include <boost/config.hpp>
#if !defined(__CYGWIN__) && !defined(BOOST_MSVC) 
#include <values.h>
#endif
#include <iostream>
#include <fstream>
#include <Lintel/AssertBoost.hpp>
#include <Lintel/Stats.hpp>


/// \brief Histogram statistics class, interface
class StatsHistogram : public Stats {

public:
    enum HistMode { Uniform, Log, UniformAccum, LogAccum };
    StatsHistogram();
    virtual ~StatsHistogram();
    virtual void reset();	// Put everything back to initial values


    //----Record a new value
    virtual void add(const double value) = 0;
    virtual void add(const Stats &stat);
    virtual void add(const double index_value, const double data_value) = 0;

    //----Histogram-wide access functions
    virtual double binWidth(unsigned index) const = 0;
    virtual double binWidth() const { return binWidth(0); }
    virtual double low() const = 0;
    virtual double binlow(unsigned index) const = 0;
    virtual double bincenter(unsigned index) const = 0;
    virtual double high() const = 0;
    virtual double binhigh(unsigned index) const = 0;
    virtual unsigned numBins() const = 0;
    virtual unsigned numRescales() const = 0;
    virtual unsigned numGrows() const = 0;
    virtual bool isScalable() const = 0;
    virtual bool isGrowable() const = 0;
    virtual double mode() const = 0;
    virtual const std::string getType() const = 0;
    virtual void printTabular(int depth, std::ostream &out) const;
    virtual void printHistogramRandomInput(std::ostream& out) const;
    virtual double percentile(double p) const = 0;   // 0.0 <=p<= 1.0
    virtual unsigned long operator[](unsigned index) const = 0;
				// returns the count in a particular bin


    //----Value-access functions
    virtual std::string debugString() const = 0;

    //----Sometimes we want another one of these, just like this one, but fresh.
    virtual Stats *another_new() const = 0;
};




/// \brief Histogram statistics class, uniformly-sized bins
class StatsHistogramUniform : public StatsHistogram {
protected:
    unsigned       num_bins;	// number of bins
    double        bin_width;	// width of each bin
    double        bin_low;	// lower bound of lowest bin
    double        bin_high;	// upper bound of highest bin
    unsigned long *bins;	// array of bin-counters
    const bool    is_scalable;	// true if rescale on outlier, false if fixed
    const bool    is_growable;	// true if grow on outlier, false if fixed
    unsigned       num_rescales;// number of times rescaled
    unsigned       num_grows;	// number of times rescaled

    unsigned sampleBin(const double x) const;

public:
    StatsHistogramUniform(const unsigned bins_in,
		   const double   low_in,
		   const double   high_in,
		   const bool     scalable = false,
		   const bool     growable = false);
    virtual ~StatsHistogramUniform();
    virtual void reset();	// Put everything back to initial values


    //----Record a new value
    virtual void add(const double value);
    virtual void add(const double index_value, const double data_value);
    using StatsHistogram::add;

    //----Histogram-wide access functions
    double binWidth() const { return bin_width; }
    double binWidth(unsigned /*index*/) const { return bin_width; }
    double   low()          const { return bin_low; }
    double binlow(unsigned index) const { return bin_low + index*bin_width; };
    double bincenter(unsigned index) const { return bin_low + (index+0.5)*bin_width; };
    double   high()         const { return bin_high; }
    double binhigh(unsigned index) const { return bin_low + (index+1)*bin_width; };
    unsigned  numBins() const { return num_bins; }
    unsigned  numRescales()  const { return num_rescales; }
    unsigned  numGrows()  const { return num_grows; }
    bool     isScalable()   const { return is_scalable; }
    bool     isGrowable()   const { return is_growable; }
    double   mode() const;
    double   percentile(double p) const;   // 0.0 <= p <= 1.0
    unsigned long  operator[](unsigned index) const;
				// Count in a particular bin


    //----Value-access functions
    virtual std::string debugString() const;

    virtual const std::string getType() const { return std::string("uniform");}
    virtual void printRome(int depth, std::ostream &out) const;

    //----Sometimes we want another one of these, just like this one, but fresh.
    virtual Stats *another_new() const;
};

/// \brief Histogram statistics class, exponentially-sized bins
class StatsHistogramLog : public StatsHistogram {
protected:
    unsigned	  num_bins;	// number of bins
    double        smallest_bin;	// width of bin zero
    double        bin_high;	// highest bound of highest bin
    double        bin_scaling;	// scales log(smallest-bin count) to bin#
    const bool    is_scalable;	// true if rescale on outlier, false if fixed
    unsigned	  num_rescales; // number of times rescaled
    unsigned long *bins;	// array of bin-counters

    unsigned sampleBin(double x) const;
    double binOffset(double index) const;

public:
    StatsHistogramLog(const unsigned bins_in,
		   const double   low_in,
		   const double   high_in,
                   const bool     scalable_in = false);
    virtual ~StatsHistogramLog();
    virtual void reset();	// Put everything back to initial values


    //----Record a new value
    virtual void add(const double value);
    virtual void add(const double index_value, const double data_value);
    using StatsHistogram::add;

    //----Histogram-wide access functions
    double binWidth() const { return binOffset(1.0); }
    double binWidth(unsigned index) const { return binOffset(index+1)-binOffset(index); }
    double low() const { return smallest_bin; }
    double binlow(unsigned index) const { return binOffset(index);};
    double bincenter(unsigned index) const {return binOffset(index+0.5);};
    double high() const { return bin_high; };
    double binhigh(unsigned index) const { return binOffset(index+1); };
    unsigned  numBins() const { return num_bins; }
    unsigned  numRescales()  const { return 0; }
    bool     isScalable()   const { return false; }
    unsigned  numGrows()  const { return 0; }
    bool     isGrowable()   const { return false; }
    double   mode() const;
    double   percentile(double p) const;   // 0.0 <= p <= 1.0
    unsigned long  operator[](unsigned index) const;  // Count in a particular bin

    //----Value-access functions
    virtual std::string debugString() const;

    virtual const std::string getType() const { return std::string("log");}
    virtual void printRome(int depth, std::ostream &out) const;
    //----Sometimes we want another one of these, just like this one, but fresh.
    virtual Stats *another_new() const;
};



/// \brief Histogram statistics class, exponentially-sized bins, sum as well as count
///
/// a kind of histogram that has exponentially-sized bins, and that
/// accumulates a value in the bin as well as the hit count.
///
/// The histogram can accumulate either the index value, using
/// the add(index_value) method, or some other data value associated
/// with the index value, using the add(index_value, data_value) method.  
/// For example, the latter form makes it possible to accumulate response 
/// times (data values) as a function of jump distance (index values).

class StatsHistogramLogAccum : public StatsHistogramLog {
private:
    double       *val_bins;

public:
    StatsHistogramLogAccum(const unsigned bins_in,
		   const double   low_in,
		   const double   high_in);
    virtual ~StatsHistogramLogAccum();
    virtual void reset();	// Put everything back to initial values

    //----Record a new value
    virtual void add(const double value);
    virtual void add(const double index_value, const double data_value);
    using StatsHistogram::add;

    //----Histogram-wide access functions
    double value(unsigned index) const;	// value in a particular bin

    //----Value-access functions
    virtual void printRome(int depth, std::ostream &out) const;

    //----Sometimes we want another one of these, just like this one, but fresh.
    virtual Stats *another_new() const;
};








/// \brief Histogram statistics class, exponentially-sized bins, sum as well as count
///
/// a kind of histogram that has uniformly-sized bins, and that
/// accumulates a value in the bin as well as the hit count.
///
/// The histogram can accumulate either the index value, using
/// the add(index_value) method, or some other data value associated
/// with the index value, using the add(index_value, data_value) method.  
/// For example, the latter form makes it possible to accumulate response 
/// times (data values) as a function of jump distance (index values).
class StatsHistogramUniformAccum : public StatsHistogramUniform {
private:
    double       *val_bins;

public:
    StatsHistogramUniformAccum(const unsigned bins_in,
		   const double   low_in,
		   const double   high_in,
   	           const bool     scalable_in = false,
   	           const bool     growable_in = false);
    virtual ~StatsHistogramUniformAccum();
    virtual void reset();	// Put everything back to initial values

    //----Record a new value
    virtual void add(const double value);
    virtual void add(const double index_value, const double data_value);
    using StatsHistogramUniform::add;

    //----Histogram-wide access functions
    double value(unsigned index) const;	// value in a particular bin

    //----Value-access functions
    virtual void printRome(int depth, std::ostream &out) const;

    //----Sometimes we want another one of these, just like this one, but fresh.
    virtual Stats *another_new() const;
};

/// \brief Histogram with specified subranges and different histogram types for each range.
/// 
/// StatsHistogramGroup: a collection of histograms, allowing multiple 
/// granularities of histograms over different ranges plus a low, high,
/// and overall stats.
///
/// The histograms may be all the same type, or of different types,
/// depending on the constructor method chosen. For example, a uniform histogram
/// around 0 and exponential above 100.
class StatsHistogramGroup : public Stats {
public:
    // ranges=[0,1,10,100] => low, hist[0..1[, hist[1..10[, hist[10,100[, high
    StatsHistogramGroup(const StatsHistogram::HistMode mode, 
                        const int sub_hist_buckets,
                        const std::vector<double> &ranges); 
    StatsHistogramGroup(const std::vector<StatsHistogram::HistMode> &_modes, 
                        const std::vector<int> &sub_hist_buckets,
                        const std::vector<double> &_ranges); 
    virtual ~StatsHistogramGroup();
    virtual void reset();	// Put everything back to initial values

    //----Record a new value
    virtual void add(const double value);
    virtual void add(const double index_value, const double data_value);
    virtual void add(const Stats &stats);
  
    const StatsHistogram &getHistogram(unsigned int n) const {
        INVARIANT(n < histograms.size(),
                  boost::format("Requested out of bounds histogram %d valid range [%d .. %d]")
                  % n % 0 % histograms.size());
        return *histograms[n];
    }

    const Stats &getLow() const { return low; }
    const Stats &getHigh() const { return high; }
    virtual void printRome(int depth, std::ostream &out) const;
  
private:
    std::vector<StatsHistogram *> histograms;
    const std::vector<StatsHistogram::HistMode> modes;
    const std::vector<double> ranges;
    Stats low,high;
};

#endif /* _LINTEL_HISTOGRAM_H_INCLUDED */







