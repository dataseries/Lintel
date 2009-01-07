/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    test for deque
*/

#include <iostream>

#include <Lintel/Deque.hpp>
using namespace std;
using boost::format;

void testPushBack() {
    Deque<int> deque;

    SINVARIANT(deque.empty());
    deque.reserve(8);
    SINVARIANT(deque.empty() && deque.capacity() == 8);
    for(int i = 0; i < 5; ++i) {
	deque.push_back(i);
	SINVARIANT(deque.back() == i);
	SINVARIANT(deque.size() == static_cast<size_t>(i + 1));
    }

    for(int i = 0; i < 5; ++i) {
	SINVARIANT(deque.front() == i);
	deque.pop_front();
	deque.push_back(i);
	SINVARIANT(deque.back() == i);
	SINVARIANT(deque.size() == 5);
    }

    {
	Deque<int>::iterator i = deque.begin(); 
	int j = 0;
	while(i != deque.end()) {
	    INVARIANT(*i == j, format("%d != %d") % *i % j);
	    ++i;
	    ++j;
	}
    }
	
    vector<int> avec;
    for(int i = 5; i < 10; ++i) {
	avec.push_back(i);
    }
    deque.push_back(avec);
    
    for(int i = 0; i < 10; ++i) {
	SINVARIANT(deque.front() == i);
	deque.pop_front();
    }
    SINVARIANT(deque.empty());
}

int main(int argc, char *argv[]) {
    testPushBack();
    cout << "deque tests passed.\n";
}

