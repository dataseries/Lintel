/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Deque implementation that has amortized constant-time performance,
    rather than log(n) 
*/

#ifndef __DEQUE_H
#define __DEQUE_H

#include <Lintel/LintelAssert.H>
#include <new>
// don't use deque for runnable; it has horrid performance on
// HP-UX (~10us / insertion to empty deque) Instead, standard
// rules on a rotating vector, valid range is [runnable_front
// .. runnable_back -1] (wrapping), one slot is wasted, front ==
// back -> empty

template <class T>
class Deque {
public:
    Deque(unsigned default_size = 8) 
	: q_front(0), q_back(0), q_size(default_size), is_resizeable(true) {
	AssertAlways(q_size > 1,("Must have size at least 2\n"));
	deque = new T[q_size];
    }

    typedef Deque<T> mytype;

#if ENABLE_SHMEM
    // Allocating it in this way means you should not! delete the
    // returned value, and it is not resizeable.
    static mytype *newSHM(SHMAlloc &shmem, int final_size) {
	void *_ret = shmem.alloc(sizeof(mytype));
	
	mytype *ret = new(_ret) Deque(shmem, final_size);
	AssertAlways(ret == _ret && ret->is_resizeable == false,
		     ("placement new didn't work correctly??\n"));
	return ret;
    }
#endif
	
    T &front() {
	Assert(2, !empty()); 
	return deque[q_front];
    }
    T &back() {
	Assert(2, !empty());
	return deque[(q_back + q_size - 1) % q_size];
    }
    void pop_front() {
	Assert(2, !empty());
	q_front = (q_front + 1) % q_size;
    }
    bool push_back(const T &val) { // returns false if unable to push on because it's full
	unsigned back_next = (q_back + 1) % q_size; 
	if (back_next != q_front) {
	    deque[q_back] = val;
	    q_back = back_next;
	    return true;
	} else { 
	    push_expand(val);
	    return true;
	}
    }
    bool empty() const { return q_front == q_back;} ;
    unsigned size() const { return (q_back + q_size - q_front) % q_size; }
    void reserve(int new_size) {
	AssertAlways(is_resizeable,("This deque isn't resizeable!\n"));
	AssertAlways(empty(),("Lame implementation only does reserve when empty\n"));
	AssertAlways(new_size > 0, ("Must have size at least 1\n"));
	new_size += 1; // for the empty entry that always exists
	q_front = q_back = 0;
	q_size = new_size;
	delete [] deque;
	deque = new T[q_size];
    }
    int capacity() const { return q_size - 1; }

    class iterator {
    public:
	iterator(Deque &_mydeque, unsigned pos) 
	    : mydeque(_mydeque), cur_pos(pos)
	{ }
	iterator &operator++() { increment(); return *this; }
	iterator operator++(int) {
	    iterator tmp = *this;
	    increment();
	    return tmp;
	}
	bool operator==(const iterator &y) const {
	    return &mydeque == &y.mydeque &&
		cur_pos == y.cur_pos;
	}
	bool operator!=(const iterator &y) const {
	    return &mydeque != &y.mydeque ||
		cur_pos != y.cur_pos;
	}
	T &operator *() {
	    AssertAlways(cur_pos >= 0 && cur_pos < mydeque.q_size,
			 ("invalid use of iterator\n"));
	    return mydeque.deque[cur_pos];
	}
    private:
	void increment() {
	    AssertAlways(cur_pos >= 0 && cur_pos < mydeque.q_size,
			 ("invalid use of iterator\n"));
	    cur_pos = (cur_pos + 1) % mydeque.q_size;
	}
	Deque &mydeque;
	unsigned cur_pos;
    };

    iterator begin() {
	return iterator(*this,q_front);
    }

    iterator end() {
	return iterator(*this,q_back);
    }
	
private:
    friend class iterator;
#if ENABLE_SHMEM
    Deque(SHMAlloc &shmem, int final_size) 
	: q_front(0), q_back(0), q_size(final_size), is_resizeable(false) {
	deque = (T *)shmem.alloc(sizeof(T) * final_size);
    }
#endif
    void push_expand(const T &val) {
	AssertAlways(is_resizeable,("this deque is not resizeable, and is full with %d items\n",q_size));
	T *new_deque = new T[q_size * 2];
	unsigned i = 0;
	// copy back to 0 offset
	if (q_front == 0) {
	    // special case if it's already 0 offset
	    for(;i<q_back;i++) {
		new_deque[i] = deque[i];
	    }
	} else {
	    for(unsigned j=q_front; j<q_size;++j) {
		new_deque[i] = deque[j];
		++i;
	    }
	    for(unsigned j=0;j<q_back;++j) {
		new_deque[i] = deque[j];
		++i;
	    }
	}
	AssertAlways(i == q_size-1,("%u %u %u %u\n",i,q_size,
				   q_front, q_back));
	q_front = 0;
	new_deque[i] = val;
	q_back = i + 1;
	q_size = q_size * 2;
	delete [] deque;
	deque = new_deque;
    }
    T *deque; 
    unsigned q_front; 
    unsigned q_back;
    unsigned q_size;

    bool is_resizeable;
};

void DequeTest();

#endif

