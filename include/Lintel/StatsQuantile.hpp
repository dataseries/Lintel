/* -*-C++-*- */
/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Non-sampled quantile statistics

    Based on the paper:
    "Approximate medians and other quantiles in one pass with limited memory"
    by G. Manku and S. Rajagopalan and B. Lindsay
    Proceedings of the ACM SIGMOD, 1998. 
*/

#ifndef LINTEL_STATSQUANTILE_HPP
#define LINTEL_STATSQUANTILE_HPP

#include <stdio.h>
#include <Lintel/Stats.hpp>
#define STATSQUANTILE_TIMING 0
#if STATSQUANTILE_TIMING
#include <Lintel/Clock.hpp>
#endif

#include <boost/utility.hpp>

class StatsQuantile : public Stats, boost::noncopyable {
public:
    /// The default tuning parameters use up about 59 KiB/StatsQuantile
    /// and support up to a billion inputs with quantile 0.9 as
    /// guarenteed between 0.89 and 0.91, and potentially much better.
    /// In practice, the actual error appears to be 2-3x better on the
    /// absolute worst number, and about 10x better on average 
    ///
    /// quantile_error means that if with quantile(phi) = the element
    /// at position ceil(phi * N) in the sorted list indexed from
    /// 1..n, then the position of the returned element is between
    /// ceil((phi - quantile_error) * N) and ceil((phi +
    /// quantile_error) * N) in the sorted list.  if phi = 0.5, then
    /// the value is a median.  If phi == 0, then the quantile the
    /// element at position 1.  Note that this indexing is different
    /// than the standard C++ indexing of arrays from 0. Nbound has to
    /// be larger than N (# elements actually added) for the quantile
    /// error to be guarenteed.
    ///
    /// Here are some statistics on the memory usage of various
    /// quantile errors and Nbounds.  Approximately increasing the
    /// nbound by 10x increases the memory usage by 1.15x, but
    /// decreasing the quantile error by 10x increases the memory usage
    /// by 7x
    ///
    /// \verbatim
    /// quantile_error  Nbound  Approx Memory Usage (MiB)
    ///        0.0001    1e8      2.096 MiB
    ///        0.0001    1e9      2.825 MiB
    ///        0.0001    1e10     4.542 MiB
    ///        0.0001    1e11     5.835 MiB
    ///        0.0001    1e12     7.022 MiB
    ///        0.0001    1e13     8.919 MiB
    /// 	  			     
    ///        0.001     1e8      0.282 MiB
    ///        0.001     1e9      0.454 MiB
    ///        0.001     1e10     0.584 MiB
    ///        0.001     1e11     0.702 MiB
    ///        0.001     1e12     0.892 MiB
    ///        0.001     1e13     1.112 MiB
    /// 	  			     
    ///        0.01      1e8      0.045 MiB
    ///        0.01      1e9      0.058 MiB
    ///        0.01      1e10     0.070 MiB
    ///        0.01      1e11     0.089 MiB
    ///        0.01      1e12     0.111 MiB
    ///        0.01      1e13     0.130 MiB
    /// \endverbatim
    StatsQuantile(double quantile_error = 0.01, 
		  long long Nbound = (long long)1000 * 1000 * 1000, 
		  int print_nrange=10);

    // this function is only here for some of the regression testing,
    // the previous one should probably always be used.  The
    // type_disambiguate string is only there because otherwise a call
    // to the constructor that specifies (double, int) is ambiguous
    StatsQuantile(const std::string &type_disambiguate, 
		  int nbuffers, int buffer_size, int print_nrange = 10);

    virtual ~StatsQuantile();
    virtual void reset();
    
    virtual void add(const double value);
    // add(Stats &) is linear in the number of values added to stat,
    // and may double the error bounds.
    virtual void add(const Stats &stat); 

    double getQuantile(double quantile) const;

    int getBufferSize() { return buffer_size; }
    int getNBuffers() { return nbuffers; }

    void dumpState(); // for debugging :(
#if STATSQUANTILE_TIMING
    Clock::T accum_gq_all, accum_gq_init, accum_gq_search, accum_gq_inner;
    int accum_gq_nelem;
#endif
    void setNrange(int _print_nrange) { print_nrange = _print_nrange; }

    // nranges is the number of ranges the printed quantiles will divide
    // [min .. max] into, so to get 10%, 20%, 30%..., nranges = 10
    void printFile(FILE *out, int nranges=-1); 
    // prints the 90% quantile, 95%, 99%, 99.5%, 99.9%, ...
    void printTail(FILE *out); 
    virtual void printRome(int depth, std::ostream &out) const;

    /// calls printTextRanges, printTextTail with default arguments
    virtual void printText(std::ostream &out) const;

    /// nranges specifies how many quantile values to print out.  The
    /// multiplier allows you to convert the units of the data during
    /// printing, for example to be able to print data in both MB/s
    /// and Mbps for network data.
    void printTextRanges(std::ostream &out, int nranges=-1, double multiplier = 1.0) const;

    /// multiplier is the same as for printTextRanges.
    void printTextTail(std::ostream &out, double multiplier = 1.0) const;

    /// How much memory will this StatsQuantile use? 
    size_t memoryUsage() const;
private:
    // add for only the quantile portion
    void addQuantile(const double value);

    typedef double *one_buffer;
    int collapseFindFirstBuffer();
    // returns total weight across these buffers, also initializes collapse_pos
    int64_t collapseSortBuffers(int first_buffer);
    int64_t collapseNextQuantileOffset(int64_t total_weight);

    void collapse();

    const double quantile_error;

    int buffer_size, nbuffers;

    /// Do initial buffer allocation and field setting. 
    void init(int _nbuffers, int _buffer_size);

    /// Zero-clear all the buffer contents for quantile computation.
    void init_buffers(); 

    // TODO: replace these with vectors, make structure with all buffer
    // related thing, e.g. weight, level, sorted(probably)

    one_buffer *all_buffers;
    // int64 is necessary for very big counts, with small buffers
    // (~100 elements at 0.1 epsilon, ~1000 at 0.01 epsilon), we could
    // overflow an int32 at 200 billion - 2 trillion added values.
    int64_t *buffer_weight; 
    int *buffer_level; // int is sufficient, goes up by one each time we collapse
    int cur_buffer, cur_buffer_pos; // cur_buffer_pos points to the current empty position in cur_buffer
    bool *buffer_sorted; // makes output faster by eliminating the need for
    // re-sorting

    // stuff for implementing collapse easier, ought to be able to get away
    // with putting tmp_buffer on the stack or some-such, but who cares
    one_buffer tmp_buffer; 
    int *collapse_pos;
    bool collapse_even_low;
    int print_nrange;

    double collapseVal(int buffer) const {
	return all_buffers[buffer][collapse_pos[buffer]];
    }
};

#endif
