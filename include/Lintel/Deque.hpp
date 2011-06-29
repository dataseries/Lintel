/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief Header file for Deque class, a double-ended queue.
*/

#ifndef LINTEL_DEQUE_HPP
#define LINTEL_DEQUE_HPP

#include <vector>

#include <boost/utility.hpp>

#include <Lintel/AssertBoost.hpp>
// don't use deque for runnable; it has horrid performance on
// HP-UX (~10us / insertion to empty deque) Instead, standard
// rules on a rotating vector, valid range is [q_front
// .. q_back -1] (wrapping), one slot is wasted, front ==
// back -> empty

/*!
    \class Deque
    \brief Double ended queue, standard array implementation
    \param T
    \par Type Requirements T
        T needs to be copy constructable
    \param Alloc
    \par Type Requirements Alloc
        An allocator, defaults to std::allocator<T>

    Deque implementation that has amortized constant-time performance,
    rather than log(n); g++ deque looks better since it does the linked list
    of multiple elements; performance comparison unclear.
*/
template <class T, class Alloc = std::allocator<T> >
class Deque : boost::noncopyable {
public:
    Deque(size_t default_size = 8) 
	: q_front(0), q_back(0), q_size(default_size) {
	INVARIANT(q_size > 1, "Must have size at least 2\n");
	deque = allocator.allocate(q_size);
    }

    ~Deque() {
	allocator.deallocate(deque, q_size);
    }
    typedef Deque<T> mytype;

    T &front() {
	DEBUG_INVARIANT(!empty(), "front() on empty deque"); 
	return deque[q_front];
    }
    T &back() {
	DEBUG_INVARIANT(!empty(), "back() on empty dequeue");
	return deque[(q_back + q_size - 1) % q_size];
    }
    T &at(size_t pos) {
	DEBUG_INVARIANT(pos < size(), "invalid position for at()");
	return deque[(q_front + pos) % q_size];
    }
    void pop_front() {
	DEBUG_INVARIANT(!empty(), "pop_front() on empty dequeue");
	allocator.destroy(deque + q_front);
	q_front = (q_front + 1) % q_size;
    }
    void push_back(const T &val) { 
	size_t back_next = (q_back + 1) % q_size; 
	if (back_next != q_front) {
	    allocator.construct(deque + q_back, val);
	    q_back = back_next;
	} else { 
	    push_expand(val);
	}
    }
    /// Add in an entire vector of values onto the back.
    void push_back(const std::vector<T> &vals) {
	typedef typename std::vector<T> vectorT;
	typedef typename std::vector<T>::const_iterator vector_iterator;
	for(vector_iterator i = vals.begin(); 
	    i != vals.end(); ++i) {
	    push_back(*i);
	}
    }

    bool empty() const { return q_front == q_back;} ;
    size_t size() const { return (q_back + q_size - q_front) % q_size; }

    // TODO: Implement correct resize (reserve + push_back's, or pop_back's if we handle non-empty)
    void reserve(size_t new_size) {
	INVARIANT(empty(),"Lame implementation only does reserve when empty");
	INVARIANT(new_size > 0 && new_size < std::numeric_limits<size_t>::max(), 
                  "Must have size at least 1, and at most size_t::max() - 1");
	new_size += 1; // for the empty entry that always exists
	q_front = q_back = 0;
	allocator.deallocate(deque, q_size);
	q_size = new_size;
	deque = allocator.allocate(q_size);
    }

    size_t capacity() const { return q_size - 1; }

    /// \brief an iterator for Deque's
    class iterator {
    public:
        // TODO-reviewer: I have defined minimum set of operator to make Deque functional with
        // std::sort function. Should we define remaining interface (as assignment operator) of
        // Random access iterators
        //
        // TODO-nitin: Eric: Yes, seems worth doing, best case is we find something in the stl
        // algorithms that requires it; also please add the doxygen documentation to all of the
        // new operations (and any old ones you feel like doing), even though said documentation
        // is mostly boilerplate.
	typedef std::forward_iterator_tag iterator_category;

	typedef T value_type;
	typedef T &reference;
	typedef T *pointer;
	typedef ptrdiff_t difference_type;

	iterator(Deque *_mydeque, size_t pos) 
	    : mydeque(_mydeque), cur_pos(pos)
	{ }
	iterator &operator++() { increment(); return *this; }
	iterator operator++(int) {
	    iterator tmp = *this;
	    increment();
	    return tmp;
	}
	bool operator==(const iterator &y) const {
	    DEBUG_SINVARIANT(mydeque == y.mydeque);
	    return cur_pos == y.cur_pos;
	}
	bool operator!=(const iterator &y) const {
	    DEBUG_SINVARIANT(mydeque == y.mydeque);
	    return cur_pos != y.cur_pos;
	}
	T &operator *() {
	    DEBUG_INVARIANT(cur_pos < mydeque->q_size, "invalid use of iterator");
	    return mydeque->deque[cur_pos];
	}
	T *operator ->() {
	    return &(operator *());
	}

        iterator &operator--() { decrement(); return *this; }
        iterator operator--(int) {
            iterator tmp = *this;
            decrement();
            return tmp;
        }

        iterator operator+(ptrdiff_t diff) {
            DEBUG_SINVARIANT(logicalOffset(*this) + diff >= 0 // <= size iterator at ::end is ok
                             && logicalOffset(*this) + diff <= mydeque->size());
            return iterator(mydeque, static_cast<size_t>(cur_pos + diff) % mydeque->q_size);
        }

        iterator operator-(ptrdiff_t diff) {
            DEBUG_SINVARIANT(logicalOffset(*this) - diff >= 0 // <= size iterator at ::end is ok
                             && logicalOffset(*this) - diff <= mydeque->size());
            return iterator(mydeque, static_cast<size_t>(cur_pos + mydeque->q_size 
                                                         - diff) % mydeque->q_size);
        }

        ptrdiff_t operator-(const iterator &rhs) const {
            DEBUG_INVARIANT(mydeque == rhs.mydeque, "invalid use of iterator");
            return logicalOffset(*this) - logicalOffset(rhs);
        }

        bool operator<(const iterator &rhs) {
            DEBUG_INVARIANT(mydeque == rhs.mydeque, "invalid use of iterator");
            return logicalOffset(*this) < logicalOffset(rhs);
        }

    private:
	void increment() {
	    DEBUG_INVARIANT(cur_pos < mydeque->q_size, "invalid use of iterator");
	    cur_pos = (cur_pos + 1) % mydeque->q_size;
	}
        void decrement() {
            DEBUG_INVARIANT(logicalOffset(*this) > 0, "invalid use of iterator");
            cur_pos = (cur_pos + mydeque->q_size - 1) % mydeque->q_size;
        }
        // The logical offset in the deque, as if the deque was a vector starting at 0.
        inline size_t logicalOffset(const iterator &i) const {
            return (i.cur_pos - i.mydeque->q_front + i.mydeque->q_size) % i.mydeque->q_size;
        }
	Deque *mydeque;
	size_t cur_pos;
    };

    iterator begin() {
	return iterator(this, q_front);
    }

    iterator end() {
	return iterator(this, q_back);
    }

    void clear() {
	if (!empty()) {
	    size_t max = q_back > q_front ? q_back : q_size;
	    for(size_t i = q_front; i < max; ++i) {
		allocator.destroy(deque + i);
	    }
	    if (q_back < q_front) {
		for(size_t i = 0; i < q_back; ++i) {
		    allocator.destroy(deque + i);
		}
	    }
	}
	q_front = q_back = 0;
	DEBUG_SINVARIANT(empty());
    }

    size_t memoryUsage() {
	return q_size * sizeof(T) + sizeof(Deque<T>);
    }

private:
    friend class iterator;
    void push_expand(const T &val) {
	T *new_deque = allocator.allocate(q_size * 2);
	size_t i = 0;
	// copy back to 0 offset
	if (q_front == 0) {
	    // special case if it's already 0 offset
	    for(;i<q_back;i++) {
		allocator.construct(new_deque + i, deque[i]);
		allocator.destroy(deque + i);
	    }
	} else {
	    for(size_t j=q_front; j<q_size;++j) {
		allocator.construct(new_deque + i, deque[j]);
		allocator.destroy(deque + j);
		++i;
	    }
	    for(size_t j=0;j<q_back;++j) {
		allocator.construct(new_deque + i, deque[j]);
		allocator.destroy(deque + j);
		++i;
	    }
	}
	INVARIANT(i == q_size-1, boost::format("internal %u %u %u %u")
                  % i % q_size % q_front % q_back);
	q_front = 0;
	allocator.construct(new_deque + i, val);
	q_back = i + 1;
	allocator.deallocate(deque, q_size);
	q_size = q_size * 2;
	deque = new_deque;
    }
    Alloc allocator;
    T *deque; 
    size_t q_front; 
    size_t q_back;
    size_t q_size;
};

void DequeTest();

#endif

