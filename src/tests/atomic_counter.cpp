#include <iostream>

#include <Lintel/AtomicCounter.hpp>
#include <Lintel/PThread.hpp>


// Basically, do the same operations to correct_count and incorrect_count and
// we expect correct_count to work and incorrect_count to be affected by races.
// Note: In simulated mode, there will be no data races.
lintel::AtomicCounter correct_count(0);
int32_t incorrect_count(0);

// Spin barrier -- be wary!
volatile int32_t doStart(0);


class AtomicCounterThread : public PThread {
public:
    AtomicCounterThread(int32_t _num_iters, int32_t _num_threads, 
			int32_t _thread_num) 
	: num_iters(_num_iters), num_threads(_num_threads), 
	  thread_num(_thread_num), final_sum(0) 
    { }

    virtual ~AtomicCounterThread() { }
    
    virtual void * run() {
	// Spin barrier -- be wary.
	while(!doStart) {
	    sched_yield();
	}

	for(int32_t i=0; i<num_iters; i++){
            // Excercise both increment interfaces: fetchThenAdd and
            // fetchThenInc
	    if(thread_num==1) {
		correct_count.fetchThenInc();
		incorrect_count++;
	    } else {
		// Split up the increment operation to vastly increase the
		// odds of a data race.
		int32_t val = incorrect_count;
		correct_count.fetchThenAdd(thread_num);
		val += thread_num;
		incorrect_count = val;
	    }
            
	    SINVARIANT(thread_num==0 || !correct_count.isZero());
	}
	return 0;
    }

    int32_t num_iters, num_threads, thread_num, final_sum;
};

// TODO-sprint-3 (0.25 day): This should be unnecessary. Remove once
// program options is clean.
void init(const std::vector<std::string> &args){
}

int main( ){
    int32_t num_threads = 250;
    int32_t num_iters = 10000;
    // Determine "the answer" for final sum.
    int32_t final_sum =0;
    for(int32_t i=0; i<num_threads; i++){
	final_sum += i;
    }
    final_sum *= num_iters; // "the answer"
    
    PThread **thread_list = new PThread*[num_threads];
    for(int32_t i=0; i<num_threads; i++){
	thread_list[i] = 
	    new AtomicCounterThread(num_iters, num_threads, i);
    }

    // Run them all at once
    for(int32_t i=0; i<num_threads; i++){
	thread_list[i]->start();
    }
    doStart=1;
    for(int32_t i=0; i<num_threads; i++){
	thread_list[i]->join();
    }

    SINVARIANT(correct_count.fetchThenAdd(0) == final_sum);

    // Theoretically, this could fail, but with such vanishingly low
    // probability (especially with the summation split the way it is)
    // that I'm not going to hold my breath.
    SINVARIANT(incorrect_count != final_sum);

    std::cout << incorrect_count << " vs " << final_sum << 
	"(" << incorrect_count / (final_sum / 100.0) << "%)" << std::endl;
    
}
