/*
   (c) Copyright 2012, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Interpolation search template
*/

#ifndef LINTEL_INTERPOLATION_SEARCH_HPP
#define LINTEL_INTERPOLATION_SEARCH_HPP

#include <inttypes.h>

#include <boost/type_traits/make_unsigned.hpp>

#include <Lintel/AssertBoost.hpp>

// http://www.cs.technion.ac.il/~itai/publications/Algorithms/p550-perl.pdf
// http://en.wikipedia.org/wiki/Interpolation_search

namespace lintel {

    // TODO: redo the interface to have no precondition, i.e. v does not have to be in first..last
    // and that if it is outside that rande, the function can return a value < 0 or > last_pos.
    // but it needs to not crash.  Then clamp the return in the caller using the standard signed
    // compare trick to usually only take one compare to check if 0..last_pos but if it is out of
    // range a second to decide whether we clamp to 0 or last_pos.  A good estimator will estimate
    // <0 if v < first, and >last_pos if v > last.  This approach means that we avoid the compares
    // to re-establish the pre-condition at the cost of compares to clamp the range, but the clamp
    // compares are always going to be fast because they are ssize_t compares, whereas the current
    // compares could be expensive because they are value compares.  Tricky question on what
    // happens if first == last --> value_range == 0.  That can't happen right now because it 
    // implies the return value is first and we will have stopped, but with this change it can
    // happen.
template<typename ValueT> struct EstimateOffset {
    typedef typename boost::make_unsigned<ValueT>::type UnsignedValueT;
    // Preconditions: first < v <= last
    // Postconditions: 0 <= return value <= last_pos
    size_t 
    operator()(const ValueT &v, const ValueT &first, const ValueT &last, size_t last_pos) const {
        UnsignedValueT value_range = last - first;
        double relative_pos = static_cast<double>(v - first) / value_range; // [0,1]
        return relative_pos * last_pos;
    }
};

template<typename ValueT> struct Compare {
    bool operator()(const ValueT &a, const ValueT &b) const {
        return a < b;
    }
};

namespace detail {

template<size_t interpolation_steps, size_t binary_steps, size_t binary_search_below_length,
         typename ValueT, typename Iterator, class EstimateOffsetT, class CompareT>
Iterator interpolationLowerBound(Iterator begin, Iterator end, const ValueT &v, 
                                 const EstimateOffsetT &estimator, const CompareT &comparer) {
    size_t estimated_offset;
    Iterator estimated_pos;

    if (begin == end) {
        return begin;
    }
    if (comparer(*begin, v)) {
        if (comparer(*(end - 1), v)) {
            // v > *end
            return end;
        } else {
            // ok, v \in ]*begin, *end]
        }
    } else {
        return begin;
    }

    // estimateOffset function can now safely assume that v is in range, so we don't have
    // to check each time to see if we are in bounds.
    
    size_t len = end - begin; // - on struct iterator involves divide, do it once.
    while (len > 0) {
        for (size_t is = 0; len > 0 && is < interpolation_steps; ++is) {
            DEBUG_SINVARIANT(begin < end);
            DEBUG_SINVARIANT((end - begin) == static_cast<ptrdiff_t>(len));
            DEBUG_SINVARIANT(comparer(*begin, v)); // *begin < v
            DEBUG_INVARIANT(!comparer(*(end - 1), v),// v <= *(end - 1)
                            boost::format("%s < %s") % *(end - 1) % v); 
            estimated_offset = estimator(v, *begin, *(end - 1), len - 1);
            //            std::cout << boost::format("len = %d, is = %d, eo = %d\n") % len % is % estimated_offset;
            estimated_pos = begin + estimated_offset;
            DEBUG_INVARIANT(estimated_offset < len, boost::format("%d >= %d from %s [%s .. %s]")
                            % estimated_offset % len % v % *begin % *(end - 1));
            DEBUG_SINVARIANT(estimated_pos < end);
            if (comparer(*estimated_pos, v)) {
                begin = estimated_pos;
                ++begin;
                if (!comparer(*begin, v)) {
                    return begin; // this is lower bound since it is not < and one less is
                }
                len = len - (estimated_offset + 1);
            } else { // estimated_pos >= v
                if (comparer(*(estimated_pos - 1), v)) {
                    return estimated_pos; // this is the lower bound since it is not < and one less is
                }
                end = estimated_pos;
                len = estimated_offset;
            }
        }

        if (len == 0) {
            return begin;
        }

        // std::cout << boost::format("len = %d\n") % len;
        if (len <= binary_search_below_length) {
            return std::lower_bound(begin, end, v, comparer);
        }
        bool adjusted_begin = false, adjusted_end = false;
        for (size_t bs = 0; len > 0 && bs < binary_steps; ++bs) {
            DEBUG_SINVARIANT(begin < end);
            DEBUG_SINVARIANT((end - begin) == static_cast<ptrdiff_t>(len));
            estimated_offset = len >> 1;
            estimated_pos = begin + estimated_offset;
            if (comparer(*estimated_pos, v)) {
                begin = estimated_pos;
                ++begin;
                len = len - (estimated_offset + 1);
                adjusted_begin = true;
            } else {
                len = estimated_offset;
                end = estimated_pos;
                adjusted_end = true;
            }
        }
        // Repair invariants used for interpolation function
        if (adjusted_begin) {
            if (!comparer(*begin, v)) {
                return begin;
            }
        }
        if (adjusted_end) {
            if (comparer(*(end - 1), v)) {
                return end;
            }
        }
    }
    return begin;
}

} // namespace detail

// We want to write estimator = EstimateOffset<ValueT>, but that requires C++0x
template<typename ValueT, typename Iterator> Iterator
interpolationLowerBound(Iterator begin, Iterator end, const ValueT &v) {
    EstimateOffset<ValueT> eo;
    return detail::interpolationLowerBound<2,4,256>(begin, end, v, eo, Compare<ValueT>());
}

} // namespace lintel

#endif
