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

void testPushBack() {
    Deque<int> deque;

    for(int i = 0; i < 5; ++i) {
	deque.push_back(i);
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
}

// TODO: test the rest of deque.
int main(int argc, char *argv[]) {
    testPushBack();
    cout << "deque tests passed.\n";
}

