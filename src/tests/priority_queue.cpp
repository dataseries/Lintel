/* -*-C++-*-
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <iostream>
#include <queue>

#include <Lintel/MersenneTwisterRandom.hpp>
#include <Lintel/PriorityQueue.hpp>

template<class T, class U>
void check(const T &queue_1, const U &queue_2) {
    if (queue_1.empty()) {
	SINVARIANT(queue_2.empty());
    } else {
	SINVARIANT(queue_1.size() == queue_2.size() && queue_1.top() == queue_2.top());
    }
}
	   
void test_basic() {
    MersenneTwisterRandom mt;

    std::priority_queue<int> stl_queue;
    PriorityQueue<int> lintel_queue;

    for(unsigned i=0; i < 1000; ++i) {
	uint32_t v = mt.randInt();
	stl_queue.push(v);
	lintel_queue.push(v);
	check(stl_queue, lintel_queue);
    }

    while(!lintel_queue.empty()) {
	check(stl_queue, lintel_queue);
	stl_queue.pop();
	lintel_queue.pop();
    }
    check(stl_queue, lintel_queue);
}

// interleave pushes and pops
void test_random() {
    MersenneTwisterRandom mt;

    std::priority_queue<int> stl_queue;
    PriorityQueue<int> lintel_queue;

    while (lintel_queue.size() < 10000) {
	uint32_t v = mt.randInt();

	check(stl_queue, lintel_queue);
	if ((v & 0x3) == 0 && !lintel_queue.empty()) { // 1/4 chance
	    stl_queue.pop();
	    lintel_queue.pop();
	} else {
	    v = mt.randInt();
	    stl_queue.push(v);
	    lintel_queue.push(v);
	}
	check(stl_queue, lintel_queue);
    }

    while(!lintel_queue.empty()) {
	check(stl_queue, lintel_queue);
	stl_queue.pop();
	lintel_queue.pop();
    }
    check(stl_queue, lintel_queue);
}

void test_replaceTop() {
    MersenneTwisterRandom mt;

    PriorityQueue<int> lintel_queue;
    PriorityQueue<int> replace_top_queue;

    while (lintel_queue.size() < 1000) {
	uint32_t v = mt.randInt();

	check(lintel_queue, replace_top_queue);
	if ((v & 0x3) == 0 && !lintel_queue.empty()) { // 1/4 chance
	    lintel_queue.pop();
	    replace_top_queue.pop();
	} else if ((v & 0x3) == 1 && !lintel_queue.empty()) { // 1/4 chance
	    v = mt.randInt();
	    lintel_queue.pop();
	    lintel_queue.push(v);
	    replace_top_queue.replaceTop(v);
	} else {
	    v = mt.randInt();
	    lintel_queue.push(v);
	    replace_top_queue.push(v);
	}
	check(lintel_queue, replace_top_queue);
    }

    for(unsigned i = 0; i < 10000; ++i) {
	uint32_t v = mt.randInt();
	lintel_queue.pop();
	lintel_queue.push(v);
	replace_top_queue.replaceTop(v);
	check(lintel_queue, replace_top_queue);
    }	

    while(!lintel_queue.empty()) {
	check(lintel_queue, replace_top_queue);
	lintel_queue.pop();
	replace_top_queue.pop();
    }
    check(lintel_queue, replace_top_queue);
}

void test_clear() {
    PriorityQueue<int> test;
    test.push(5);
    test.clear();
    SINVARIANT(test.empty());
}

int main() {
    test_basic();
    test_random();
    test_replaceTop();
    test_clear();
    std::cout << "Test passed\n";
    return 0;
}
