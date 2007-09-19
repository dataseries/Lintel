/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Deque implementation
*/

#include <Lintel/Deque.H>

void
DequeTest()
{
    Deque<int> testq;

    Deque<int> testBad(256);
    for(int i=0;i<512;i++) {
	testBad.push_back(i);
    }
    for(int i=0;i<512;i++) {
	INVARIANT(testBad.empty() == false && testBad.front() == i,
		  "simple test failed?!");
	testBad.pop_front();
    }

    // test empty->size1->empty
    for(int size1=1;size1<2000;size1++) {
	INVARIANT(testq.empty()==true,"internal");
	for(int j=0;j<size1;j++) {
	    testq.push_back(j);
	    INVARIANT(testq.empty()==false,"internal");
	}
	for(int j=0;j<size1;j++) {
	    INVARIANT(testq.empty()==false,"internal");
	    INVARIANT(testq.front() == j,
		      boost::format("Internal Error: %d/%d")
		      % size1 % j);
	    testq.pop_front();
	}
	INVARIANT(testq.empty()==true,"internal");
    }
    printf("pass empty->full->empty tests\n");
    // test empty->size1, size2 add/remove, ->empty
    for(int size1=1;size1<150;size1++) {
	for(int size2=1;size2<400;size2++) {
	    INVARIANT(testq.empty()==true,"internal");
	    for(int j=0;j<size1;j++) {
		testq.push_back(j);
		INVARIANT(testq.empty()==false,"internal");
	    }
	    for(int j=0;j<size2;j++) {
		INVARIANT(testq.empty()==false,"internal");
		INVARIANT(testq.front() == j,
			  boost::format("Internal Error: %d/%d/%d")
			  % size1 % size2 %j);
		testq.pop_front();
		testq.push_back((j+size1));
	    }
	    for(int j=size2;j<size1+size2;j++) {
		INVARIANT(testq.empty()==false,"internal");
		int t = testq.front();
		testq.pop_front();
		INVARIANT(t == j,
			  boost::format("Internal Error: %d/%d/%d/%d")
			  % t % size1 % size2 % j);
	    }
	    INVARIANT(testq.empty()==true,"internal");
	}
    }
    printf("pass partial fill tests\n");
}

