/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Priority Queue Implementation as a heap
*/

#ifndef LINTEL_PRIORITY_QUEUE_HPP
#define LINTEL_PRIORITY_QUEUE_HPP

#include <stdint.h>

#include <Lintel/AssertBoost.hpp>
#include <new>

// The priority_queue in the HPUX STL is ~4x slower than this one on
// small numbers of things in the queue, and ~1.2x slower with lots in
// the queue for no apparent reason other than perhaps the code is way
// too complex for the optimizer to handle.  The Linux STL with gcc
// 2.95 shows the same behavior

// LessImportant(a,b) === a less_important_than b -> true; it is best
// if a is less_important_than a. priority queue returns the most
// important item first, which is consistent with how the C++ pq
// behaves

// Sample usage:
//
// struct priorityData {
//     int key;
//     double data;
// };
// 
// struct priorityDataGeq {
// bool operator()(const priorityData &a, const priorityData &b) const {
//     return a.key >= b.key;
// };
//
// PriorityQueue<priorityData,priorityDataGeq> pq;
// 
// The net effect of the above will be to return the data from smallest
// key value to largest key value from the priority queue.

template<class T, class LessImportant = std::less<T> >
class PriorityQueue {
public:
    PriorityQueue(int initial_size = 1024/sizeof(T)) 
	: size_available(initial_size), pq_size(0), resizeable(true) {
	pq = new T[initial_size];
    }

    PriorityQueue(const LessImportant &li, int initial_size = 1024/sizeof(T)) 
	: lessimportant(li), size_available(initial_size), pq_size(0), resizeable(true) {
	pq = new T[initial_size];
    }

    
    ~PriorityQueue() {
	if (resizeable) {
	    delete [] pq;
	}
	pq = NULL;
    }
    typedef PriorityQueue<T, LessImportant> mytype;

#if ENABLE_SHMEM
    // Allocating it in this way means you should not! delete the
    // returned value, and it is not resizeable.
    static mytype *newSHM(SHMAlloc &shmem, int final_size) {
	void *_ret = shmem.alloc(sizeof(mytype));
	
	mytype *ret = new(_ret) PriorityQueue(shmem, final_size);
	INVARIANT(ret == _ret && ret->resizeable == false,
		  "placement new didn't work correctly??");
	return ret;
    }
#endif
    T &top() { DEBUG_SINVARIANT(!empty()); return pq[0]; }
    const T &top() const { DEBUG_SINVARIANT(!empty()); return pq[0]; }
    
    void pop() {
	DEBUG_SINVARIANT(pq_size > 0);
	--pq_size;
	if (pq_size > 0) {
	    uint32_t cur_pos = 0;
	    while(1) {
		uint32_t down_left = 2*cur_pos + 1;
		if (down_left >= pq_size)
		    break;
		uint32_t down_right = down_left + 1;
		
		uint32_t smaller_down = down_left;
		if (down_right < pq_size &&
		    !lessimportant(pq[down_right],pq[down_left])) {
		    smaller_down = down_right;
		}
		if (!lessimportant(pq[pq_size],pq[smaller_down])) {
		    break; // done, down moving value less than both children
		}
		pq[cur_pos] = pq[smaller_down];
		cur_pos = smaller_down;
	    }
	    pq[cur_pos] = pq[pq_size];
	}
    }

    void push(const T &val) {
	if (pq_size == size_available) {
	    double_size();
	}
	int end_pos = pq_size;
	while(end_pos > 0) {
	    int up_pos = (end_pos - 1)/2;
	    if (!lessimportant(val,pq[up_pos])) {
		pq[end_pos] = pq[up_pos];
		end_pos = up_pos;
	    } else {
		break;
	    }
	}
	pq[end_pos] = val;
	++pq_size;
    }

    /// equivalent to this->pop(), this->push(val), but roughly 2x as
    /// efficient
    void replaceTop(const T &val) {
	uint32_t cur_pos = 0;
	while(1) {
	    uint32_t down_left = 2*cur_pos + 1;
	    if (down_left >= pq_size)
		break;
	    uint32_t down_right = down_left + 1;

	    // First decide which side we would swap with
	    uint32_t more_important_down = down_left;
	    if (down_right < pq_size && lessimportant(pq[down_left], pq[down_right])) {
		more_important_down = down_right;
	    }
	    if (lessimportant(val, pq[more_important_down])) {
		pq[cur_pos] = pq[more_important_down];
		cur_pos = more_important_down;
	    } else {
		break; // at final position
	    }
	}
	DEBUG_SINVARIANT(cur_pos < pq_size);
	pq[cur_pos] = val;
    }
	
    uint32_t size() const {
	return pq_size;
    }

    bool empty() const {
	return pq_size == 0;
    }
    
    void reserve(uint32_t amt) {
	INVARIANT(empty(), "Lame implementation only able to reserve empty priority queue");
		  
	INVARIANT(resizeable, "PQ isn't resizeable?");
	if (amt > pq_size) {
	    delete [] pq;
	    pq = new T[amt];
	    size_available = amt;
	}
    }
	
    // Think hard before using these.
    T *PQ_Begin() { return pq; };
    T *PQ_End() { return pq + pq_size; }
private:
#if ENABLE_SHMEM
    PriorityQueue(SHMAlloc &shmem, int final_size) 
    : size_available(final_size), pq_size(0), resizeable(false) {
	pq = (T *)shmem.alloc(sizeof(T) * final_size);
    }
#endif

    LessImportant lessimportant;
    void double_size() {
	INVARIANT(resizeable, boost::format("this PQ isn't resizeable, and only holds %d elements.") % size_available);
		  
	T *pq_new = new T[size_available * 2];
	for(unsigned i=0;i<size_available;i++) {
	    pq_new[i] = pq[i];
	}
	delete [] pq;
	pq = pq_new;
	size_available = size_available * 2;
    }
    uint32_t size_available;
    uint32_t pq_size;
    T *pq;
    bool resizeable;
    // 0 -> 1,2
    // 1 -> 3,4
    // 2 -> 5,6
    // i -> 2*i + 1, 2*i + 2

    // j -> (j-1) / 2
    // -- could leave position 0 empty to make the rules 2*i, 2*i + 1
    // -- and j -> j / 2
};

void PriorityQueueTest();

#endif
