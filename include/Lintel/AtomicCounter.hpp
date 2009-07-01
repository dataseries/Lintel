#ifndef LINTEL_ATOMIC_COUNTER_HPP
#define LINTEL_ATOMIC_COUNTER_HPP

// The tests for feature support are taken from boost 1.37

// The primitive for non 4.1 compilers was also taken from boost 1.37

// The primitive for 4.1 compilers is Joe's code, but happens to be the same
// as boost 1.37 (it's kinda obvious given the knowledge of GCC's
// primitives).

// Note that boost code is tested on other architectures, this specific code
// has only been tested on x86 with gcc4.1.

namespace lintel {

#if defined ( __GNUC__) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 401 ) && !defined(__hppa ) && ( !defined( __INTEL_COMPILER ) || defined ( __ia64__ )  )

    // GCC 4.1 and up
#define _ATOMIC_COUNTER_ADD_AND_FETCH(var, amount)      \
    __sync_add_and_fetch( &(var), (amount) )

#elif defined( __GLIBCPP__ ) || defined(__GLIBCXX__)

    // Other compilers & poorly supported architectures, e.g. PA-RISC.. :-( 

#if defined(__GLIBCXX__) // g++ 3.4+
    using __gnu_cxx::__exchange_and_add;
#endif

#define _ATOMIC_COUNTER_ADD_AND_FETCH(var, amount)      \
    __exchange_and_add( &(var),(amount) );

#else

#error AtomicCounter does not have primitives for the detected platform

#endif


    /// Encapsulates an atomic counter.  Be wary when using it so as
    /// to not inadvertently introduce a race--the value may change at
    /// any time outside of the defined operations.
    ///
    /// If you really need the current value, then fetchThenAdd(0).
    /// Beware that the returned value may immediatly become out of
    /// date!
    class AtomicCounter {
    public:
        explicit AtomicCounter(int32_t _counter=0) : counter(_counter) { }

        /// Returns the value of counter, and then adds one
        int32_t fetchThenInc() {
            return _ATOMIC_COUNTER_ADD_AND_FETCH(counter, 1);
        }
    
        /// Returns the value of the counter, and then adds amount
        int32_t fetchThenAdd(int32_t amount) {
            return _ATOMIC_COUNTER_ADD_AND_FETCH(counter, amount);
        }

        /// Returns true if the counter is zero
        int32_t isZero(){
            return 0 == _ATOMIC_COUNTER_ADD_AND_FETCH(counter, 0);
        }

    private:
        /// Copy construction is forbidden.
        AtomicCounter(AtomicCounter const &);

        /// Assignment is forbidden too.
        AtomicCounter & operator=(AtomicCounter const &);

        /// Various code elsewhere depends on the counter being 32 bits.
        int32_t counter;
    };

}

#endif
