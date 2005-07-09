/* -*-C++-*-
*******************************************************************************
*
* File:         Deque.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/Deque.C,v 1.1 2003/10/27 04:09:29 anderse Exp $
* Description:  Deque implementation
* Author:       Eric Anderson
* Created:      Fri Nov  2 09:01:14 2001
* Modified:     Sat Nov  2 22:07:48 2002 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#include <Deque.H>

void
DequeTest()
{
    Deque<int> testq;

    Deque<int> testBad(256);
    for(int i=0;i<512;i++) {
	testBad.push_back(i);
    }
    for(int i=0;i<512;i++) {
	AssertAlways(testBad.empty() == false && testBad.front() == i,
		     ("simple test failed?!\n"));
	testBad.pop_front();
    }

    // test empty->size1->empty
    for(int size1=1;size1<2000;size1++) {
	AssertAlways(testq.empty()==true,("internal\n"));
	for(int j=0;j<size1;j++) {
	    testq.push_back(j);
	    AssertAlways(testq.empty()==false,("internal\n"));
	}
	for(int j=0;j<size1;j++) {
	    AssertAlways(testq.empty()==false,("internal\n"));
	    AssertAlways(testq.front() == j,
			 ("Internal Error: %d/%d\n",size1,j));
	    testq.pop_front();
	}
	AssertAlways(testq.empty()==true,("internal\n"));
    }
    printf("pass empty->full->empty tests\n");
    // test empty->size1, size2 add/remove, ->empty
    for(int size1=1;size1<150;size1++) {
	for(int size2=1;size2<400;size2++) {
	    AssertAlways(testq.empty()==true,("internal\n"));
	    for(int j=0;j<size1;j++) {
		testq.push_back(j);
		AssertAlways(testq.empty()==false,("internal\n"));
	    }
	    for(int j=0;j<size2;j++) {
		AssertAlways(testq.empty()==false,("internal\n"));
		AssertAlways(testq.front() == j,
			     ("Internal Error: %d/%d/%d\n",size1,size2,j));
		testq.pop_front();
		testq.push_back((j+size1));
	    }
	    for(int j=size2;j<size1+size2;j++) {
		AssertAlways(testq.empty()==false,("internal\n"));
		int t = testq.front();
		testq.pop_front();
		AssertAlways(t == j,
			     ("Internal Error: %d/%d/%d/%d\n",t,size1,size2,j));
	    }
	    AssertAlways(testq.empty()==true,("internal\n"));
	}
    }
    printf("pass partial fill tests\n");
}

