#ifndef LINTEL_ATOMIC_COUNTER_HPP
#define LINTEL_ATOMIC_COUNTER_HPP

/** @file
    \brief Header file for lintel::Atomic class
*/

//http://www.justsoftwaresolutions.co.uk/threading/intel-memory-ordering-and-c++-memory-model.html

#include <inttypes.h>

#if defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 401) // need at least gcc 4.1
#    if defined(__i386__) // Pure __i386__ does not have a __sync_add_and_fetch that can return a value.
#        if defined(__i486__) || defined(__i586__) || defined(__i686__)
#            define LINTEL_HAS_SYNC_ADD_AND_FETCH 1
#        endif       
#    else // Assume all non i386 archs have it, (__i386__ is not defined on __x86_64) 
#        define LINTEL_HAS_SYNC_ADD_AND_FETCH 1
#    endif
#endif

//#define LINTEL_USE_STD_ATOMICS 1
//#define LINTEL_USE_GCC_BUILTIN_SYNC_ATOMICS 1
#define LINTEL_USE_GCC_ASM_ATOMICS 1

#if defined (LINTEL_USE_STD_ATOMICS)
//#    include <cstddef>
#    include <atomic>
#endif

namespace lintel {

#if defined (LINTEL_USE_STD_ATOMICS)
#    define LINTEL_ATOMIC_FETCH(op, var, amount)  ::std::atomic_fetch_##op(var, amount)
#    define LINTEL_ATOMIC_LOAD(ptr) ::std::atomic_load(ptr)
#    define LINTEL_ATOMIC_STORE(ptr, val) ::std::atomic_store(ptr, val)
#    define LINTEL_ATOMIC_EXCHANGE(ptr, val) ::std::atomic_exchange(ptr, val)
#    define LINTEL_COMPARE_EXCHANGE(current, expected, desired) ::std::atomic_compare_exchange_strong(current, expected, desired)
#    define LINTEL_ATOMIC_THREAD_FENCE(order) ::std::atomic_thread_fence(order)
#elif defined (LINTEL_USE_GCC_BUILTIN_SYNC_ATOMICS)
#    define LINTEL_ATOMIC_FETCH(op, var, amount)  ::__sync_fetch_and_##op(var, amount)
#    define LINTEL_ATOMIC_LOAD(ptr) ({::__sync_synchronize(); __typeof(*ptr) t=*ptr; ::__sync_synchronize(); t;})//sorta guranteed to be atomic
#    define LINTEL_ATOMIC_STORE(ptr, val) {*ptr=val; ::__sync_synchronize();}
#    define LINTEL_ATOMIC_EXCHANGE(ptr, val) ::__sync_lock_test_and_set(ptr, val)
#    define LINTEL_COMPARE_EXCHANGE(current, expected, desired) ({__typeof(*expected) val=*expected; val==(*expected=::__sync_val_compare_and_swap(current, val, desired));})
#    define LINTEL_ATOMIC_THREAD_FENCE(order) {::__sync_synchronize();(void)order;}
#elif defined (LINTEL_USE_GCC_ASM_ATOMICS)
#    define LINTEL_ATOMIC_FETCH(op, var, amount)  lintel::x86Gcc_atomic_fetch_##op(var, amount)
#    define LINTEL_ATOMIC_LOAD(ptr) (*ptr)
#    define LINTEL_ATOMIC_STORE(ptr, val) lintel::x86Gcc_atomic_store(ptr, val)
#    define LINTEL_ATOMIC_EXCHANGE(ptr, val) lintel::x86Gcc_atomic_exchange(ptr, val)
#    define LINTEL_COMPARE_EXCHANGE(current, expected, desired) lintel::x86Gcc_compare_exchange(current, expected, desired)
#    define LINTEL_ATOMIC_THREAD_FENCE(order) lintel::x86Gcc_atomic_thread_fence(order)
#endif

#if defined (LINTEL_USE_STD_ATOMICS)
  using ::std::memory_order;
  using ::std::memory_order_relaxed;
  using ::std::memory_order_consume;
  using ::std::memory_order_acquire;
  using ::std::memory_order_release;
  using ::std::memory_order_acq_rel;
  using ::std::memory_order_seq_cst;
#else 
  /// Enumeration for memory_order
  typedef enum memory_order {
    memory_order_relaxed, memory_order_consume, memory_order_acquire,
    memory_order_release, memory_order_acq_rel, memory_order_seq_cst
  } memory_order;
#endif

#if defined(LINTEL_USE_GCC_ASM_ATOMICS)

    template<typename T>
    static inline void x86Gcc_atomic_store(T *counter, T v) {
      asm ("xchg %1, %0": "+m" (*counter), "+r" (v)::);
    }
    template<typename T>
    static inline T x86Gcc_atomic_exchange(T *counter, T v) {
      asm ("xchg %1, %0": "+m" (*counter), "+r" (v)::);
      return v;
    }
    template<typename T>
    static inline bool x86Gcc_compare_exchange(T* current, T* expected, T desired) {
      bool result;
      asm ("lock; cmpxchg %3,%0\n\t"
	   "setz %2\n\t"
	   : "+m" (*current), "+a" (*expected), "=r"(result)
	   : "r" (desired)
	   : );
      return result;
    }

    template<typename T>
    static inline T x86Gcc_atomic_fetch_add(T *counter, T v) {
      asm ("lock xadd %1, %0": "+m" (*counter), "+r" (v)::);
      return v;
    }
    template<typename T>
    static inline T x86Gcc_atomic_fetch_sub(T *counter, T v) {
      return x86Gcc_atomic_fetch_add(counter, -v);
    }
    template<typename T>
    static inline T x86Gcc_atomic_fetch_or(T *counter, T v) {
        T expected = LINTEL_ATOMIC_LOAD(counter);
	T desired;
	do {
	    desired = expected | v;
	} while (!x86Gcc_compare_exchange(counter, &expected, desired));
	return expected;
    }
    template<typename T>
    static inline T x86Gcc_atomic_fetch_and(T *counter, T v) {
        T expected = LINTEL_ATOMIC_LOAD(counter);
	T desired;
	do {
	    desired = expected & v;
	} while (!x86Gcc_compare_exchange(counter, &expected, desired));
	return expected;
    }
    template<typename T>
    static inline T x86Gcc_atomic_fetch_xor(T *counter, T v) {
        T expected = LINTEL_ATOMIC_LOAD(counter);
	T desired;
	do {
	    desired = expected ^ v;
	} while (!x86Gcc_compare_exchange(counter, &expected, desired));
	return expected;
    }

    static inline void x86Gcc_atomic_thread_fence(memory_order order)
    {
        switch(order) {
	case memory_order_acquire:
	case memory_order_consume:
	    asm volatile ("lfence":::"memory");
	    break;
	case memory_order_release:
	    asm volatile ("sfence":::"memory");
	    break;
	case memory_order_acq_rel:
	case memory_order_seq_cst:
	    asm volatile ("mfence":::"memory");
	    break;
        case memory_order_relaxed:
	    break; // do nothing
      }
    }


 #if 0
  // Generates same assembly as __sync_add_and_fetch on lenny 64bit
  // (Except for the #APP #NO_APP bits)
    static inline T x86Gcc_atomic_fetch_add(T *counter, T v) {
	//xadd below could be more explictit - xaddb xaddw xaddl xaddq, but gasm
	//uses correct one automatically, since it knows argument size
      asm /*volatile*/("lock; xadd %1, %0; \n"
		     : "+m" (*counter), "+r" (v)
		     : // above specs list input+output
		     : //"memory" // clobber memory (+volatile = compiler barrier)
		       // without memory clobber, generated asm is closer to
		       // __sync_add_and_fetch builtin, which is not a total barrier, but
		       // only a barrier for *counter memory location
		     );
	return v;
    }
#endif

#endif

#if defined(LINTEL_ATOMIC_COUNTER_ADD_THEN_FETCH)
    // pre-defined by user
#elif defined (LINTEL_USE_STD_ATOMICS) \
   || defined (LINTEL_USE_GCC_ASM_ATOMICS) \
   || defined (LINTEL_USE_GCC_BUILTIN_SYNC_ATOMICS)
#    define LINTEL_ATOMIC_COUNTER_ADD_THEN_FETCH(counter, amount) \
  ((amount) + LINTEL_ATOMIC_FETCH(add, &(counter), (amount)))
  /*__sync_add_and_fetch(&(var),(amount))*/
#else
#error Atomic does not have primitives for the detected platform
#endif

    extern "C" void atomic_thread_fence(memory_order order)
    { LINTEL_ATOMIC_THREAD_FENCE(order); }

    /// \brief An atomic counter that avoids using locks.
    ///
    /// Encapsulates an atomic counter.  Be wary when using it so as
    /// to not inadvertently introduce a race--the value may change at
    /// any time outside of the defined operations.
    ///
    /// If you really need the current value, then fetchThenAdd(0).
    /// Beware that the returned value may immediatly become out of
    /// date!
    template<class T>
    class Atomic {
    public:
        //FIXME (maybe): in std::atomic this must be deault constructor, but isn't
        explicit Atomic(T counter = 0) : counter(counter) { }

        /// Increments the counter and then returns the value
        /// C11/C++11 have no such function. Use prefix operator++() instead
        T incThenFetch() { return ++*this; }

	/// Decrements the counter and then returns the value
        /// C11/C++11 have no such function. Use prefix operator--() instead
	T decThenFetch() { return --*this; }

        /// Adds amount to the counter and then returns the value
        /// C11/C++11 have no such function. Use operator += instead
        T addThenFetch(T amount) {
            return LINTEL_ATOMIC_COUNTER_ADD_THEN_FETCH(counter, amount);
        }

        /// Returns true if the counter is zero
        /// C11/C++11 have no such function. Use operator T() instead
        bool isZero() const { return !this->load(); }

        operator T() const { return load(); }

        T load() const {
	    return LINTEL_ATOMIC_LOAD(&counter);
	    //return counter;
	    //return const_cast<Atomic*>(this)->fetch_add(0); //make simple load
        }
        void store(T t) {
	    LINTEL_ATOMIC_STORE(&counter, t);
        }

        /// Assignement
        T operator=(T amount) { store(amount); return amount; }

        T exchange(T t) {
	    return LINTEL_ATOMIC_EXCHANGE(&counter, t);
        }

        bool compare_exchange_strong(T * expected, T desired)
        { return LINTEL_COMPARE_EXCHANGE(&counter, expected, desired); }

        T fetch_add(T amount) { return LINTEL_ATOMIC_FETCH(add, &counter, amount); }
        T fetch_sub(T amount) { return LINTEL_ATOMIC_FETCH(sub, &counter, amount); }
        T fetch_or (T amount) { return LINTEL_ATOMIC_FETCH(or , &counter, amount); }
        T fetch_and(T amount) { return LINTEL_ATOMIC_FETCH(and, &counter, amount); }
        T fetch_xor(T amount) { return LINTEL_ATOMIC_FETCH(xor, &counter, amount); }

        T operator +=(T amount) { return fetch_add(amount) + amount; }
        T operator -=(T amount) { return fetch_sub(amount) - amount; }
        T operator |=(T amount) { return fetch_or (amount) | amount; }
        T operator &=(T amount) { return fetch_and(amount) & amount; }
        T operator ^=(T amount) { return fetch_xor(amount) ^ amount; }

        T operator++(int) { return this->fetch_add(1);  } //suffix
        T operator--(int) { return this->fetch_sub(1);  }
        T operator++(   ) { return this->fetch_add(1)+1;} //prefix
        T operator--(   ) { return this->fetch_sub(1)-1;}

    private:
        /// Copy construction is forbidden.
        Atomic(const Atomic&); //=delete

        /// Copy assignment is forbidden.
        Atomic& operator=(const Atomic&); // = delete;

#if defined (LINTEL_USE_STD_ATOMICS)
        std::atomic<T> counter;
#elif defined (LINTEL_USE_GCC_BUILTIN_SYNC_ATOMICS) || defined (LINTEL_USE_GCC_ASM_ATOMICS)
        /*volatile*/ T counter; //BSD stdatomic.h uses volatile, GCC atomic doesn't
#endif
    };


    // For backward compatibility
    typedef Atomic<int32_t> AtomicCounter; // Unclear if mandatory

    //typedef Atomic<int32_t> Atomic_int32_t; // C++11 does not have this because C11 doesn't

    typedef Atomic<int_fast8_t > Atomic_int_fast8_t;
    typedef Atomic<int_fast16_t> Atomic_int_fast16_t;
    typedef Atomic<int_fast32_t> Atomic_int_fast32_t;
    typedef Atomic<int_fast64_t> Atomic_int_fast64_t;
    typedef Atomic<size_t> Atomic_size_t;
    typedef Atomic<intmax_t> Atomic_intmax_t;
}

#endif
