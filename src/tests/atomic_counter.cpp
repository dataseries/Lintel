#include <iostream>
#include <vector>

#include <Lintel/PThread.hpp>
#include <Lintel/AtomicCounter.hpp>

// Basically, do the same operations to correct_count and incorrect_count and
// we expect correct_count to work and incorrect_count to be affected by races.
// Note: In simulated mode, there will be no data races.

// Spin barrier -- be wary!
static volatile int32_t do_start(0);

template<typename IntXX_T>
class AtomicThread : public PThread {
public:
    static lintel::Atomic<IntXX_T> correct_count;
    static IntXX_T incorrect_count;
public:
    AtomicThread(int32_t num_iters, int32_t num_threads, int32_t thread_num) 
	: num_iters(num_iters), num_threads(num_threads), thread_num(thread_num), final_sum(0) 
    { }

    virtual ~AtomicThread() { }
    
    virtual void *run() {
	// Spin barrier -- be wary.
        while (!do_start) {
	    sched_yield();
	}

        for(int32_t i=0; i < 1000; ++i) {
            IntXX_T val = incorrect_count;
            correct_count.incThenFetch();
            sched_yield(); // Almost guarantee a failure
            incorrect_count = val + 1;
        }

	for (int32_t i=0; i<num_iters; ++i) {
            // Excercise both increment interfaces: fetchThenAdd and
            // fetchThenInc
	    if (thread_num == 0) {
                IntXX_T val = incorrect_count;
		correct_count.incThenFetch();
                incorrect_count = val + 1;
	    } else {
		// Split up the increment operation to vastly increase the
		// odds of a data race.
		IntXX_T val = incorrect_count;
		correct_count.addThenFetch(thread_num+1);
		val += thread_num;
		incorrect_count = val;
	    }
	}
	return 0;
    }

    int32_t num_iters;
    IntXX_T num_threads, thread_num, final_sum;
};

template<typename IntXX_T> lintel::Atomic<IntXX_T> AtomicThread<IntXX_T>::correct_count(0);
template<typename IntXX_T> IntXX_T AtomicThread<IntXX_T>::incorrect_count(0);

template <typename IntXX_T>
void simpleTest() {
    std::cout << "testing int" << sizeof(IntXX_T)*CHAR_BIT << std::endl;

    lintel::Atomic<IntXX_T> bar;
    bar.store(3); SINVARIANT(bar.load() == 3);
    SINVARIANT(bar.fetch_add(5)==3); SINVARIANT(bar.load()==8);
    SINVARIANT(bar.fetch_sub(5)==8); SINVARIANT(bar.load()==3);

    IntXX_T ibar;
    lintel::unsafe::atomic_store(&ibar, 3); SINVARIANT(lintel::unsafe::atomic_load(&ibar) == 3);
    SINVARIANT(lintel::unsafe::atomic_fetch_add(&ibar, 5)==3); SINVARIANT(lintel::unsafe::atomic_load(&ibar)==8);
    SINVARIANT(lintel::unsafe::atomic_fetch_sub(&ibar, 5)==8); SINVARIANT(lintel::unsafe::atomic_load(&ibar)==3);

    bar=7; SINVARIANT(bar==7);
    SINVARIANT(bar.exchange(9)==7); SINVARIANT(bar==9);

    ibar=7; SINVARIANT(lintel::unsafe::atomic_load(&ibar)==7);
    SINVARIANT(lintel::unsafe::atomic_exchange(&ibar, 9)==7); SINVARIANT(lintel::unsafe::atomic_load(&ibar)==9);

    bar=15; IntXX_T expected = 15, desired = 17;
    SINVARIANT(bar.compare_exchange_strong(&expected, desired)); SINVARIANT(expected==15);
    bar=19;
    SINVARIANT(!bar.compare_exchange_strong(&expected, desired)); SINVARIANT(expected==19);

    ibar=15; expected = 15, desired = 17;
    SINVARIANT(lintel::unsafe::atomic_compare_exchange_strong(&ibar, &expected, desired)); SINVARIANT(expected==15);
    ibar=19;
    SINVARIANT(!lintel::unsafe::atomic_compare_exchange_strong(&ibar, &expected, desired)); SINVARIANT(expected==19);


    bar = 0; SINVARIANT(bar.fetch_or(0) == 0); SINVARIANT(bar==0);
    bar = 0; SINVARIANT(bar.fetch_or(1) == 0); SINVARIANT(bar==1);
    bar = 1; SINVARIANT(bar.fetch_or(0) == 1); SINVARIANT(bar==1);
    bar = 1; SINVARIANT(bar.fetch_or(1) == 1); SINVARIANT(bar==1);

    bar = 0; SINVARIANT(bar.fetch_and(0) == 0); SINVARIANT(bar==0);
    bar = 0; SINVARIANT(bar.fetch_and(1) == 0); SINVARIANT(bar==0);
    bar = 1; SINVARIANT(bar.fetch_and(0) == 1); SINVARIANT(bar==0);
    bar = 1; SINVARIANT(bar.fetch_and(1) == 1); SINVARIANT(bar==1);

    bar = 0; SINVARIANT(bar.fetch_xor(0) == 0); SINVARIANT(bar==0);
    bar = 0; SINVARIANT(bar.fetch_xor(1) == 0); SINVARIANT(bar==1);
    bar = 1; SINVARIANT(bar.fetch_xor(0) == 1); SINVARIANT(bar==1);
    bar = 1; SINVARIANT(bar.fetch_xor(1) == 1); SINVARIANT(bar==0);

    bar=0; SINVARIANT(0==(bar|=0));
    bar=0; SINVARIANT(1==(bar|=1));
    bar=1; SINVARIANT(1==(bar|=0));
    bar=1; SINVARIANT(1==(bar|=1));

    bar=0; SINVARIANT(0==(bar&=0));
    bar=0; SINVARIANT(0==(bar&=1));
    bar=1; SINVARIANT(0==(bar&=0));
    bar=1; SINVARIANT(1==(bar&=1));

    bar=0; SINVARIANT(0==(bar^=0));
    bar=0; SINVARIANT(1==(bar^=1));
    bar=1; SINVARIANT(1==(bar^=0));
    bar=1; SINVARIANT(0==(bar^=1));

    ibar = 1; SINVARIANT(lintel::unsafe::atomic_fetch_or (&ibar,0) == 1); SINVARIANT(lintel::unsafe::atomic_load(&ibar)==1);
    ibar = 1; SINVARIANT(lintel::unsafe::atomic_fetch_and(&ibar,1) == 1); SINVARIANT(lintel::unsafe::atomic_load(&ibar)==1);
    ibar = 1; SINVARIANT(lintel::unsafe::atomic_fetch_xor(&ibar,1) == 1); SINVARIANT(lintel::unsafe::atomic_load(&ibar)==0);

    lintel::atomic_thread_fence(lintel::memory_order_relaxed);
    lintel::atomic_thread_fence(lintel::memory_order_consume);
    lintel::atomic_thread_fence(lintel::memory_order_acquire);
    lintel::atomic_thread_fence(lintel::memory_order_release);
    lintel::atomic_thread_fence(lintel::memory_order_acq_rel);
    lintel::atomic_thread_fence(lintel::memory_order_seq_cst);

    lintel::Atomic<IntXX_T> foo(0);
    IntXX_T x = foo.load();
    INVARIANT(x == 0, boost::format("initial value is %d, not 0") % x);
    x = foo.incThenFetch();
    INVARIANT(x == 1, boost::format("inc-then-fetch -> %d ; cur -> %d") % x % foo.load());
    foo.addThenFetch(-1);
    SINVARIANT(foo.isZero());
    SINVARIANT(foo.incThenFetch() == 1);
    SINVARIANT(!foo.isZero());
    SINVARIANT(foo.addThenFetch(-1) == 0);
    SINVARIANT(foo.isZero());
    std::cout << "simple test ok\n";
}

template <typename IntXX_T>
void doTest () {
    simpleTest<IntXX_T>();

    // For this test, running on more than # cores doesn't have any
    // benefit since our races are entirely from running threads.
    // on very few cpus, having one extra thread increases our chance of a race
    int32_t num_threads = PThreadMisc::getNCpus(true) + 1;
    if (num_threads <= 4) {
        num_threads = 4; // should be enough to get some races
    }
    int32_t num_iters = 1000 * 1000; // 1 million chances to screw up.

    // Determine "the answer" for final sum.
    IntXX_T final_sum = 1000 * num_threads; // initial loop with sched_yield
    for(int32_t i=0; i<num_threads; ++i) {
	final_sum += (i+1) * num_iters; // each thread adds a different amount
    }
    
    std::vector<PThread *> thread_list;
    for(int32_t i=0; i<num_threads; ++i) {
        thread_list.push_back(new AtomicThread<IntXX_T>(num_iters, num_threads, i));
    }

    // Run them all at once
    for(int32_t i=0; i<num_threads; ++i) {
	thread_list[i]->start();
    }
    do_start = 1;
    for(int32_t i=0; i<num_threads; ++i) {
	thread_list[i]->join();
    }

    SINVARIANT(AtomicThread<IntXX_T>::correct_count.load() == final_sum);

    // Theoretically, this could fail, but with such vanishingly low
    // probability (especially with the summation split the way it is)
    // that I'm not going to hold my breath.
    SINVARIANT(AtomicThread<IntXX_T>::incorrect_count != final_sum);

    std::cout << AtomicThread<IntXX_T>::incorrect_count << " vs " << final_sum << "(" 
	      << AtomicThread<IntXX_T>::incorrect_count / (final_sum / 100.0) << "%)"
	      << std::endl;
}

int main()
{
  doTest<uint8_t>();
  doTest<uint16_t>();
  doTest<int32_t>();
  doTest<int64_t>();
}
