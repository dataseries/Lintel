/* -*-C++-*-
   (c) Copyright 2003-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Priority queue implementation
*/

#include <vector>
#include <queue>
#include <cstdlib>

#include <Lintel/LintelAssert.H>
#include <Lintel/MersenneTwisterRandom.H>
#include <Lintel/PriorityQueue.H>

class IntCompare {
public:
    bool operator()(const int &a, const int &b) {
	return a >= b;
    }
};

#if 0
extern "C" {
    void mal_debug(int);
    void mal_verify(int);
}
#endif

void
PriorityQueueTest()
{
    PriorityQueue<int,IntCompare> intq(1000);

    for(int i=1;i<1000;i++) {
	for(int j=0;j<i;j++) {
	    AssertAlways(intq.size() == j,("bad\n"));
	    intq.push(j);
	}
	AssertAlways(intq.size() == i,("bad\n"));
	for(int j=0;j<i;j++) {
	    AssertAlways(intq.top() == j,("bad\n"));
	    intq.pop();
	}
	AssertAlways(intq.empty(),("bad\n"));
    }
    printf("completed testing in-order insertion\n");
    for(int i=1;i<1000;i++) {
	for(int j=i;j>0;j--) {
	    AssertAlways(intq.size() == i-j,("bad %d %d\n",i,j));
	    intq.push(j-1);
	}
	AssertAlways(intq.size() == i,("bad\n"));
	for(int j=0;j<i;j++) {
	    AssertAlways(intq.top() == j,("bad %d %d %d\n",i,j,intq.top()));
	    intq.pop();
	}
	AssertAlways(intq.empty(),("bad\n"));
    }
    printf("completed testing reverse-order insertion\n");
    for(int step = 2;step<6;step++) {
	for(int i=1;i<1000;i++) {
	    for(int start=0;start<step;start++) {
		for(int j=start;j<i;j += step) {
		    intq.push(j);
		}
	    }
	    AssertAlways(intq.size() == i,("bad %d %d\n",i,intq.size()));
	    for(int j=0;j<i;j++) {
		AssertAlways(intq.top() == j,
			     ("bad %d %d %d\n",i,j,intq.top()));
		intq.pop();
	    }
	    AssertAlways(intq.empty(),("bad\n"));
	}
	printf("completed step-wise insertion at step %d\n",step);
    }
    for(int i = 100;i < 500000;i *= 2) {
      	for(int j=i;j>0;j--) {
	    AssertAlways(intq.size() == i-j,("bad %d %d\n",i,j));
	    intq.push(j-1);
	}
#if 0
	mal_verify(1);
#endif
	AssertAlways(intq.size() == i,("bad\n"));
	for(int j=0;j<i;j++) {
	    AssertAlways(intq.top() == j,("bad %d %d %d\n",i,j,intq.top()));
	    intq.pop();
	}
	AssertAlways(intq.empty(),("bad\n"));
#if 0
	mal_verify(1);
#endif
    }
    printf("completed size doubling insertion\n");

    std::priority_queue<int, std::vector<int>, IntCompare> cpp_intq;
    for(int rep = 0;rep<100;rep++) {
	for(int i=0;i<10000;i++) {
	    int v = MTRandom.randInt();
	    intq.push(v);
	    cpp_intq.push(v);
	    int v1 = intq.top();
	    int v2 = cpp_intq.top();
	    AssertAlways(v1 == v2,("Whoa, %d (mypq) != %d (cpp pq)\n",v1, v2));
	}
	for(int i=0;i<10000;i++) {
	    int v1 = intq.top();
	    int v2 = cpp_intq.top();
	    AssertAlways(v1 == v2,("Whoa, %d (mypq) != %d (cpp pq)\n",v1, v2));
	    AssertAlways(intq.size() == (int)cpp_intq.size(),
			 ("Whoa size difference?!\n"));
	    intq.pop();
	    cpp_intq.pop();
	}
    }
    printf("completed comparison to cpp priority queue\n");
}

#ifdef PQ_MAIN
int
main()
{
    char * foo = (char *)std::malloc(5);
#if MV
    mal_verify(1);
    mal_debug(3);
#endif
    PriorityQueueTest();
#if MV
    mal_verify(1);
#endif
}
#endif

