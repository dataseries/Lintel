/* -*-C++-*-
   (c) Copyright 2003-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    HashTable test program
*/

#include <stdio.h>

#include <Lintel/ConstantString.H>
#include <Lintel/HashTable.H>
#include <Lintel/HashMap.H>

// // test that hash table work done at init time is correct
// commented out for now; don't feel like getting it to interact properly
// with autoconf and the only sometimes building constantstring
// ConstantString foo("foo");
// ConstantString bar("bar");

using namespace std;

HashMap<string,string> test;

class intHash {
public:
    unsigned int operator()(const int k) {
	return k;
    }
};

class collisionHash {
public:
    unsigned int operator()(const int k) {
	return 0;
    }
};

class intEqual {
public:
    bool operator()(const int a, const int b) {
	return a == b;
    }
};

void
stringHTTests()
{
    HashMap<string, unsigned> test;

    for(unsigned i = 0; i < 1000; ++i) {
	char buf[30];
	sprintf(buf,"%d", i);
	
	test[string(buf)] = i;
    }

    for(unsigned i = 0; i < 1000; ++i) {
	char buf[30];
	sprintf(buf,"%d", i);
	
	AssertAlways(test[string(buf)] == i, ("bad"));
    }

}

int main()
{
//    // Testing for hash table work done at init time...
//    AssertAlways(strcmp(foo.c_str(),"foo")==0 &&
//		 strcmp(bar.c_str(),"bar")==0,("internal error\n"));
    printf("test collision add/remove\n");
    
    HashTable<int,collisionHash,intEqual> collisiontable;
    for(int i=0;i<100;i++) {
	collisiontable.add(i);
    }
    for(int i=0;i<100;i+=2) {
	collisiontable.remove(i);
	int count = 0;
	for (HashTable<int,collisionHash,intEqual>::iterator i = collisiontable.begin();
	     i != collisiontable.end();i++) {
	    count++;
	}
	AssertAlways(count == collisiontable.size(),
		     ("?! %d %d\n",count,collisiontable.size()));
    }
    
    int maxi = 20000;
    typedef HashTable<int,intHash,intEqual> inttable;
    inttable table;
    
    printf("test adding...\n");
    for(int i=0;i<maxi;i++) {
	AssertAlways(table.size() == i,("?!"));
	table.add(i);
    }
    for(int i=0;i<maxi;i++) {
	AssertAlways(table.lookup(i) != NULL,("?!"));
	AssertAlways(*table.lookup(i) == i,("?!"));
    }
    for(inttable::iterator i = table.begin();i != table.end();i++) {
	AssertAlways(*i >= 0 && *i < maxi, ("?!"));
    }
    printf("test copying...\n");
    inttable table2;
    table2 = table;
    for(int i=0;i<maxi;i++) {
	AssertAlways(table2.lookup(i) != NULL,("?!"));
	AssertAlways(*table2.lookup(i) == i,("?!"));
    }
    printf("test assigning...\n");
    inttable table3(table2);
    for(int i=0;i<maxi;i++) {
	AssertAlways(table3.lookup(i) != NULL,("?!"));
	AssertAlways(*table3.lookup(i) == i,("?!"));
    }

    printf("test removing...\n");
    for(int i=0;i<maxi/2;i++) {
	AssertAlways(table.lookup(i) != NULL,("?!"));
	AssertAlways(*table.lookup(i) == i,("?!"));
	AssertAlways(table.size() == maxi-i,("?!"));
	table.remove(i);
    }
    for(int i=maxi/2;i<maxi;i++) {
	AssertAlways(table.lookup(i) != NULL,("?!"));
	AssertAlways(*table.lookup(i) == i,("?!"));
    }
    for(inttable::iterator i = table.begin();i != table.end();i++) {
	AssertAlways(*i >= maxi/2 && *i < maxi, ("?!"));
    }
    printf("test complete remove...\n");
    for(int i=maxi/2;i<maxi;i++) {
	AssertAlways(table.size() == maxi-i,("?!"));
	table.remove(i);
    }
    AssertAlways(table.size() == 0,("?!"));
    table.clear();
    AssertAlways(table.size() == 0,("?!"));
    printf("test removing strange subset...\n");
    for(int i=0;i<10*maxi;i++) {
	table.add(i);
    }
    for(int i=0;i<10*maxi;i++) {
	if ((i % 17) > 10) {
	    table.remove(i);
	}
    }
    for(int i=0;i<10*maxi;i++) {
	const int *x = table.lookup(i);
	if ((i % 17) <= 10) {
	    AssertAlways(x != NULL && *x == i,("?!"));
	} else {
	    AssertAlways(x == NULL,("?!"));
	}
    }

    table.clear();
    AssertAlways(table.size() == 0,("?!"));
    table.remove(5,false);

    stringHTTests();

    printf("success.\n");
}

