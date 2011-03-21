#ifndef LINTEL_ATOMIC_COUNTER_HPP
#define LINTEL_ATOMIC_COUNTER_HPP

/** @file
    \brief Header file for lintel::AtomicCounter class
*/

#include <inttypes.h>

namespace lintel {

#if defined(LINTEL_ATOMIC_COUNTER_ADD_THEN_FETCH)
    // pre-defined by user
#elif defined (__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 401)

    // GCC 4.1 and up
#    define LINTEL_ATOMIC_COUNTER_ADD_THEN_FETCH(var, amount) \
            __sync_add_and_fetch(&(var), (amount))

#elif defined (__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    // Generates same assembly as __sync_add_and_fetch on lenny 64bit
    // (Except for the #APP #NO_APP bits)
    static inline int x86GccAddThenFetch(int *counter, int v) {
	int save_v = v;
	asm volatile("lock xaddl %1, %0; \n"
		     : "+m" (*counter), "+r" (v)
		     : // above specs list input+output
		     : "memory" // clobber memory
		     );
	return v + save_v; // xaddl returns the pre-updated value
    }

#    define LINTEL_ATOMIC_COUNTER_ADD_THEN_FETCH(counter, amount) \
            x86GccAddThenFetch(&(counter), (amount))
#else
#error AtomicCounter does not have primitives for the detected platform
#endif

    /// \brief An atomic counter that avoids using locks.
    ///
    /// Encapsulates an atomic counter.  Be wary when using it so as
    /// to not inadvertently introduce a race--the value may change at
    /// any time outside of the defined operations.
    ///
    /// If you really need the current value, then fetchThenAdd(0).
    /// Beware that the returned value may immediatly become out of
    /// date!
    class AtomicCounter {
    public:
        explicit AtomicCounter(int32_t counter = 0) : counter(counter) { }

        /// Increments the counter and then returns the value
        int32_t incThenFetch() {
            return addThenFetch(1);
        }

	/// Decrements the counter and then returns the value
	int32_t decThenFetch() {
	    return addThenFetch(-1);
	}

        /// Adds amount to the counter and then returns the value
        int32_t addThenFetch(int32_t amount) {
            return LINTEL_ATOMIC_COUNTER_ADD_THEN_FETCH(counter, amount);
        }

        /// Returns true if the counter is zero
        bool isZero() {
            return 0 == addThenFetch(0);
        }

    private:
        /// Copy construction is forbidden.
        AtomicCounter(AtomicCounter const &);

        /// Assignment is forbidden too.
        AtomicCounter & operator=(AtomicCounter const &);

        /// 32 bit counter works on both 32bit and 64bit machines.
        int32_t counter;
    };

}

#endif
