/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#ifndef LINTEL_TUPLES_HPP
#define LINTEL_TUPLES_HPP

#include <bitset>
#include <ostream>

#include <boost/tuple/tuple_comparison.hpp>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/HashFns.hpp>

/** @file

    \brief Extentions to boost tuples.

    Additional tuple bits to extend what is in the boost tuples.  We
    add in two new "types" of tuples, first AnyTuples, which is a
    tuple made of of pairs of booleans representing "any" and the
    actual value.  Second, BitsetAnyTuples, which are a more efficient
    version of anytuples since they store the any information in a
    single bitset.  We also add hashing for all of the various tuple
    types that interacts well with HashMap and friends.
*/

namespace boost { namespace tuples {
    // These functions need to be defined inside the boost::tuples namespace or otherwise
    // the argument based lookup + lookup at instantiation time implementation of LLVM 3.1 does
    // not find these functions.
    inline uint32_t hash(const null_type &) { return 0; }

    template<class Head>
    inline uint32_t hash(const cons<Head, null_type> &v) {
        return lintel::hash(v.get_head());
    }

    // See http://burtleburtle.net/bob/c/lookup3.c for a discussion
    // about how we could have even fewer calls to the mix function.
    // Would want to upgrade to the newer hash function.
    template<class Head1, class Head2, class Tail>
    inline uint32_t hash(const cons<Head1, cons<Head2, Tail> > &v) {
        uint32_t a = lintel::hash(v.get_head());
        uint32_t b = lintel::hash(v.get_tail().get_head());
        uint32_t c = hash(v.get_tail().get_tail());
        return lintel::BobJenkinsHashMix3(a,b,c);
    }

    template <typename T, typename U>
    inline uint32_t hashType(const cons<T, U> &v) {
        return hash(v);
    }
} }

namespace lintel {
/// \brief tuples namespace

namespace tuples {
    // these types are so integral to the implementation of lintel::tuples that it seems reasonable
    // to pull them into lintel tuples namespace.  Minimize the amount of bits that we add in to the
    // boost namespace.
    using boost::tuples::null_type;
    using boost::tuples::cons;


    template<class BitSet>
    inline uint32_t bitset_any_hash(const null_type &, const BitSet &, size_t) {
        return 0;
    }

    // 0x8bc74d0b was sampled from /dev/random. It should be  better choice than
    // 0, which is a more common value in variables.
    template<class Head, class BitSet>
    inline uint32_t bitset_any_hash(const cons<Head, null_type> &v,
                                    const BitSet &any, size_t cur_pos) {
        return any[cur_pos] ? 0x8bc74d0bU : lintel::hash(v.get_head());
    }

    template<class Head1, class Head2, class Tail, class BitSet>
    inline uint32_t bitset_any_hash(const cons<Head1, cons<Head2, Tail> > &v,
                                    const BitSet &any, size_t cur_pos) {
        uint32_t a = any[cur_pos] ? 0x8bc74d0bU : lintel::hash(v.get_head());
        uint32_t b = any[cur_pos+1] ? 0x8bc74d0bU : lintel::hash(v.get_tail().get_head());
        uint32_t c = bitset_any_hash(v.get_tail().get_tail(), any, cur_pos + 2);
        return BobJenkinsHashMix3(a,b,c);
    }

    /// \brief structure for hashing tuples
    template<class Tuple> struct TupleHash {
        uint32_t operator()(const Tuple &a) const {
            return boost::tuples::hash(a);
        }
    };

    template<class BitSet>
    inline bool bitset_any_equal(const null_type &, const null_type &,
                                 const BitSet &, size_t) {
        return true;
    }

    template<class Head, class Tail, class BitSet>
    inline bool bitset_any_equal(const cons<Head, Tail> &lhs, const cons<Head, Tail> &rhs,
                                 const BitSet &any, size_t cur_pos) {
        if (any[cur_pos] || lhs.get_head() == rhs.get_head()) {
            return bitset_any_equal(lhs.get_tail(), rhs.get_tail(), any, cur_pos + 1);
        } else {
            return false;
        }
    }

    template<class BitSet>
    inline bool bitset_any_strict_less_than(const null_type &, const null_type &,
                                            const BitSet &, const BitSet &,
                                            size_t) {
        return false;
    }

    template<class Head, class Tail, class BitSet> inline bool
    bitset_any_strict_less_than(const cons<Head, Tail> &lhs, const cons<Head, Tail> &rhs,
                                const BitSet &any_lhs, const BitSet &any_rhs,
                                size_t cur_pos) {
        if (!any_lhs[cur_pos] && !any_rhs[cur_pos]) {
            if (lhs.get_head() < rhs.get_head()) {
                return true;
            } else if (lhs.get_head() > rhs.get_head()) {
                return false;
            } else {
                // don't know, fall through to recursion.
            }
        } else if (!any_lhs[cur_pos] && any_rhs[cur_pos]) {
            return true; // used < any
        } else if (any_lhs[cur_pos] && !any_rhs[cur_pos]) {
            return false; // any > used
        }
        return bitset_any_strict_less_than(lhs.get_tail(), rhs.get_tail(),
                                           any_lhs, any_rhs, cur_pos + 1);
    }

    template<class BitSet>
    inline void bitset_any_print(std::ostream &, const null_type &, const BitSet &, size_t)
    { }

    template<class Head, class Tail, class BitSet>
    inline void bitset_any_print(std::ostream &to, const cons<Head, Tail> &v,
                                 const BitSet &any, size_t cur_pos) {
        if (cur_pos > 0) {
            to << " ";
        }
        if (any[cur_pos]) {
            to << "*";
        } else {
            to << v.get_head();
        }
        bitset_any_print(to, v.get_tail(), any, cur_pos + 1);
    }

    /// \brief class that represents "any" or a value.
    /// Not a pair so we can have operators that behave differently, and
    /// to improve clarity.
    template<typename T> struct AnyPair {
        bool any;
        T val;
        AnyPair() : any(true) { }
        AnyPair(const T &v) : any(false), val(v) { }
        void set(const T &v) { val = v; any = false; }

        bool operator ==(const AnyPair<T> &rhs) const {
            if (any != rhs.any) {
                return false;
            } else if (any) {
                return true;
            } else {
                return val == rhs.val;
            }
        }
        bool operator <(const AnyPair<T> &rhs) const {
            if (any && rhs.any) {
                return false;
            } else if (any && !rhs.any) {
                return false;
            } else if (!any && rhs.any) {
                return true;
            } else if (!any && !rhs.any) {
                return val < rhs.val;
            }
            FATAL_ERROR("?");
        }
    };

    /// \cond SEMI_INTERNAL_CLASSES
    template<class T0, class T1> struct ConsToAnyPairCons {
        typedef ConsToAnyPairCons<typename T1::head_type,
                                  typename T1::tail_type> tail;
        typedef boost::tuples::cons<AnyPair<T0>, typename tail::type> type;
    };

    template<class T0> struct ConsToAnyPairCons<T0, null_type> {
        typedef boost::tuples::cons<AnyPair<T0>, null_type> type;
    };
    /// \endcond

    /// \brief CLass for converting a tuple to an any tuple.
    template<class T> struct TupleToAnyTuple
        : ConsToAnyPairCons<typename T::head_type, typename T::tail_type>
    { };

    /// \brief value or any Tuple represented by tuple + bitset for any booleans
    template<class Tuple> struct BitsetAnyTuple {
        BOOST_STATIC_CONSTANT(uint32_t, length = boost::tuples::length<Tuple>::value);

        Tuple data;
        typedef std::bitset<boost::tuples::length<Tuple>::value> AnyT;
        AnyT any;

        BitsetAnyTuple() { }
        /// defaults to no any's
        explicit BitsetAnyTuple(const Tuple &from) : data(from) { }

        bool operator==(const BitsetAnyTuple &rhs) const {
            if (any != rhs.any) {
                return false;
            }
            return lintel::tuples::bitset_any_equal(data, rhs.data, any, 0);
        }
        bool operator<(const BitsetAnyTuple &rhs) const {
            return lintel::tuples::bitset_any_strict_less_than(data, rhs.data, any, rhs.any, 0);

        }
    };

    /// \brief class for hashing BitsetAnyTuple's
    template<class Tuple> struct BitsetAnyTupleHash {
        uint32_t operator()(const BitsetAnyTuple<Tuple> &v) const {
            return lintel::tuples::bitset_any_hash(v.data, v.used, 0);
        }
    };

    template<class T>
    inline std::ostream &operator <<(std::ostream &to, const BitsetAnyTuple<T> &val) {
        to << "(";
        lintel::tuples::bitset_any_print(to, val.data, val.any, 0);
        to << ")";
        return to;
    }

    /// Base version of assign
    void assign(null_type, null_type) { }

    /// Assign an anytuple from a normal tuple of the same type;
    /// unfortunately, we can't make this also an operator = since
    /// operator = can only be defined as part of the class.
    template<typename Head, typename AnyTail, typename Tail>
    void assign(cons<AnyPair<Head>, AnyTail> &to, const cons<Head, Tail> &from) {
        to.get_head().any = false;
        to.get_head().val = from.get_head();
        assign(to.get_tail(), from.get_tail());
    }

    template<class T> inline uint32_t hashType(const tuples::AnyPair<T> &v) {
        if (v.any) {
            // 0x8bc74d0b was sampled from /dev/random. It should be better choice than
            // 0, which is a more common value in variables.
            return 0x8bc74d0bU;
        } else {
            return hash(v.val);
        }
    }

    template<class T> inline
    uint32_t hashType(const tuples::BitsetAnyTuple<T> &v) {
        return tuples::bitset_any_hash(v.data, v.any, 0);
    }
} }

#endif
