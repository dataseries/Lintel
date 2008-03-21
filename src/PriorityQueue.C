/* -*-C++-*-
   (c) Copyright 2003-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Priority queue implementation
*/

#include <vector>
#include <queue>

#include <Lintel/LintelAssert.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>
#include <Lintel/PriorityQueue.hpp>

class IntCompare {
public:
    bool operator()(const int &a, const int &b) {
	return a >= b;
    }
};

using namespace std;
using boost::format;

void
PriorityQueueTest()
{
    PriorityQueue<int,IntCompare> intq(1000);

    for(unsigned i=1;i<1000;i++) {
	for(unsigned j=0;j<i;j++) {
	    SINVARIANT(intq.size() == j);
	    intq.push(j);
	}
	SINVARIANT(intq.size() == i);
	for(unsigned j=0;j<i;j++) {
	    SINVARIANT(intq.top() == static_cast<int>(j));
	    intq.pop();
	}
	SINVARIANT(intq.empty());
    }
    cout << "completed testing in-order insertion\n";
    for(unsigned i=1;i<1000;i++) {
	for(int j=i;j>0;j--) {
	    INVARIANT(intq.size() == i-j,format("bad %d %d") % i %j);
	    intq.push(j-1);
	}
	SINVARIANT(intq.size() == i);
	for(unsigned j=0;j<i;j++) {
	    INVARIANT(intq.top() == static_cast<int>(j),
		      format("bad %d %d %d") % i % j % intq.top());
	    intq.pop();
	}
	SINVARIANT(intq.empty());
    }
    cout << "completed testing reverse-order insertion\n";
    for(int step = 2;step<6;step++) {
	for(unsigned i=1;i<1000;i++) {
	    for(int start=0;start<step;start++) {
		for(unsigned j=start;j<i;j += step) {
		    intq.push(j);
		}
	    }
	    INVARIANT(intq.size() == i, 
		      format("bad %d %d") % i % intq.size());
	    for(unsigned j=0;j<i;j++) {
		INVARIANT(intq.top() == static_cast<int>(j),
			  format("bad %d %d %d") % i % j % intq.top());
		intq.pop();
	    }
	    SINVARIANT(intq.empty());
	}
	cout << format("completed step-wise insertion at step %d\n") % step;
    }
    for(uint32_t i = 100;i < 500000;i *= 2) {
      	for(unsigned j=i;j>0;j--) {
	    INVARIANT(intq.size() == i-j, format("bad %d %d") % i % j);
	    intq.push(j-1);
	}
	SINVARIANT(intq.size() == i);
	for(uint32_t j=0;j<i;j++) {
	    INVARIANT(intq.top() == static_cast<int>(j),
		      format("bad %d %d %d") % i % j % intq.top());
	    intq.pop();
	}
	SINVARIANT(intq.empty());
    }
    cout << "completed size doubling insertion\n";

    std::priority_queue<int, std::vector<int>, IntCompare> cpp_intq;
    for(int rep = 0;rep<100;rep++) {
	for(int i=0;i<10000;i++) {
	    int v = MTRandom.randInt();
	    intq.push(v);
	    cpp_intq.push(v);
	    int v1 = intq.top();
	    int v2 = cpp_intq.top();
	    INVARIANT(v1 == v2,
		      format("Whoa, %d (mypq) != %d (cpp pq)") % v1 % v2);
	}
	for(int i=0;i<10000;i++) {
	    int v1 = intq.top();
	    int v2 = cpp_intq.top();
	    INVARIANT(v1 == v2,
		      format("Whoa, %d (mypq) != %d (cpp pq)") % v1 % v2);
	    INVARIANT(intq.size() == cpp_intq.size(),
		      "Whoa size difference?!");
	    intq.pop();
	    cpp_intq.pop();
	}
    }
    cout << "completed comparison to cpp priority queue";
}

#ifdef PQ_MAIN
int
main()
{
    char * foo = (char *)malloc(5);

    PriorityQueueTest();
}
#endif

