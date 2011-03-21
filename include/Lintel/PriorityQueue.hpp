/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief Priority Queue Implementation as a heap
*/

#ifndef LINTEL_PRIORITY_QUEUE_HPP
#define LINTEL_PRIORITY_QUEUE_HPP

#include <stdint.h>

#include <boost/utility.hpp>

#include <Lintel/AssertBoost.hpp>

// TODO: performance test this to see if it is still needed.

/// \brief Priority queue implementation as a heap.
/// The priority_queue in the HPUX STL is ~4x slower than this one on
/// small numbers of things in the queue, and ~1.2x slower with lots in
/// the queue for no apparent reason other than perhaps the code is way
/// too complex for the optimizer to handle.  The Linux STL with gcc
/// 2.95 shows the same behavior

/// LessImportant(a,b) === a less_important_than b -> true; it is best
/// if a is less_important_than a. priority queue returns the most
/// important item first, which is consistent with how the C++ pq
/// behaves

/// Sample usage:
///
/// struct priorityData {
///     int key;
///     double data;
/// };
/// 
/// struct priorityDataGeq {
/// bool operator()(const priorityData &a, const priorityData &b) const {
///     return a.key >= b.key;
/// };
///
/// PriorityQueue<priorityData,priorityDataGeq> pq;
/// 
/// The net effect of the above will be to return the data from smallest
/// key value to largest key value from the priority queue.
template<class T, class LessImportant = std::less_equal<T> > 
class PriorityQueue : boost::noncopyable {
public:
    PriorityQueue(int initial_size = 1024/sizeof(T)) 
	: size_available(initial_size), pq_size(0) {
	pq = new T[initial_size];
    }

    PriorityQueue(const LessImportant &li, int initial_size = 1024/sizeof(T)) 
	: lessimportant(li), size_available(initial_size), pq_size(0) {
	pq = new T[initial_size];
    }

    ~PriorityQueue() {
	delete [] pq;
	pq = NULL;
    }
    typedef PriorityQueue<T, LessImportant> mytype;

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

    // TODO: look at http://www.diku.dk/~jyrki/Paper/CATS98.ps; 
    // TODO: look at http://www.mpi-inf.mpg.de/~sanders/papers/spqjea.ps.gz see
    // if the bottom up heuristic can be adapted when we don't have a supremum
    // (which we don't for example, for strings); problem is that this
    // implementation is slower than the one above, and the one they have.  Some
    // possibilities: 1) it's the 2*n + 1 calculation and we should index the
    // priority queue starting at 1.  2) It's the extra size check on the way
    // down.
    void pop_bottom_up_heuristic() {
	DEBUG_SINVARIANT(pq_size > 0);
	uint32_t hole = 0;
	uint32_t down = 1;

	while(down < pq_size) {
	    if (down + 1 == pq_size || lessimportant(pq[down+1], pq[down])) { 
		// one child, or rhs < lhs, so pick lhs
	    } else { // rhs bubbles up
		++down;
	    } 
	    pq[hole] = pq[down];
	    hole = down; 
	    down = 2*down + 1;
	}

	T &back(pq[pq_size - 1]);
	if (hole > 0) {
	    SINVARIANT(lessimportant(back, back)); // make sure loop below will terminate

	    uint32_t up = (hole - 1) / 2;
	    while (!lessimportant(back, pq[up])) { // back more important, move hole up
		pq[hole] = pq[up];
		hole = up;
		up = (hole - 1) / 2;
	    }
	}

	pq[hole] = back;
	--pq_size;
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
    
    void clear() {
	pq_size = 0;
    }

    void reserve(uint32_t amt) {
	INVARIANT(empty(), "Lame implementation only able to reserve empty priority queue");
		  
	if (amt > pq_size) {
	    delete [] pq;
	    pq = new T[amt];
	    size_available = amt;
	}
    }
	
    void selfVerify() {
	for(uint32_t i = 0; i < pq_size; ++i) {
	    uint32_t down_left = 2 * i + 1;
	    if (down_left >= pq_size) break;
	    SINVARIANT(!lessimportant(pq[i], pq[down_left]));
	    uint32_t down_right = down_left + 1;
	    if (down_right >= pq_size) break;
	    SINVARIANT(!lessimportant(pq[i], pq[down_right]));
	}
    }

    // Think hard before using these.
    T *PQ_Begin() { return pq; };
    T *PQ_End() { return pq + pq_size; }
private:
    LessImportant lessimportant;
    void double_size() {
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
