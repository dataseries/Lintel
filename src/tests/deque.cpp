/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    test for deque
*/

// TODO: add performance comparison to this test
#include <iostream>

#include <Lintel/Deque.hpp>
using namespace std;
using boost::format;

class NoDefaultConstructor {
public:
    NoDefaultConstructor(int i) : v(i) { ++ndc_count; }
    NoDefaultConstructor(const NoDefaultConstructor &i) : v(i.v) { ++ndc_count; }
    ~NoDefaultConstructor() { --ndc_count; }

    static int ndc_count;
    int v;

private:
    NoDefaultConstructor operator = (const NoDefaultConstructor &);
};

int NoDefaultConstructor::ndc_count;

void testNoDefaultConstructor() {
    Deque<NoDefaultConstructor> deque;

    deque.reserve(8);

    for(int i = 0; i < 5; ++i) {
	deque.push_back(NoDefaultConstructor(i));
	SINVARIANT(deque.back().v == i);
	SINVARIANT(deque.size() == static_cast<size_t>(i + 1));
	INVARIANT(NoDefaultConstructor::ndc_count == i + 1, format("%d != %d + 1")
		  % NoDefaultConstructor::ndc_count % i);
    }

    for(int i = 0; i < 5; ++i) {
	SINVARIANT(deque.front().v == i);
	deque.pop_front();
	deque.push_back(NoDefaultConstructor(i));
	SINVARIANT(deque.back().v == i);
	SINVARIANT(deque.size() == 5);
	SINVARIANT(NoDefaultConstructor::ndc_count == 5);
    }

    for(int i = 0; i < 5; ++i) {
	SINVARIANT(deque.front().v == i);
	deque.pop_front();
	SINVARIANT(deque.size() == static_cast<size_t>(4 - i));
	SINVARIANT(NoDefaultConstructor::ndc_count == 4 - i);
    }
    SINVARIANT(NoDefaultConstructor::ndc_count == 0);
}
    
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
	SINVARIANT(deque.at(i) == i);
    }

    for(int i = 0; i < 5; ++i) {
	SINVARIANT(deque.front() == i);
	deque.pop_front();
	deque.push_back(i);
	SINVARIANT(deque.back() == i);
	SINVARIANT(deque.size() == 5);
    }

    for(int i = 0; i < 5; ++i) {
	SINVARIANT(deque.at(i) == i);
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

void testAssign() {
    Deque<int> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);

    vector<int> b;
    b.assign(a.begin(), a.end());
    SINVARIANT(b.size() == 3);
    for(uint32_t i = 0; i < 3; ++i) {
	SINVARIANT(b[i] == i+1);
    }
}

int main(int argc, char *argv[]) {
    testPushBack();
    testNoDefaultConstructor();
    testAssign();
    cout << "deque tests passed.\n";
}

