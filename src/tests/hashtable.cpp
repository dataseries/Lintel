/* -*-C++-*-
   (c) Copyright 2003-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    HashTable test program
*/

#include <stdio.h>

#include <Lintel/HashTable.H>
#include <Lintel/HashMap.H>
#include <Lintel/AssertBoost.H>

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
	
	SINVARIANT(test[string(buf)] == i);
    }

}

typedef HashTable<int,intHash,intEqual> inttable;

void nonconstHTTest(inttable &table, int maxi) {
    vector<bool> found;
    found.resize(maxi);
    for(inttable::iterator i = table.begin();
	i != table.end(); ++i) {
	SINVARIANT(*i < maxi);
	SINVARIANT(found[*i] == false);
	found[*i] = true;
    }

    for(int i=0; i<maxi; ++i) {
	SINVARIANT(found[i]);
    }
}

void constHTTest(const inttable &table, int maxi) {
    vector<bool> found;
    found.resize(maxi);
    for(inttable::const_iterator i = table.begin();
	i != table.end(); ++i) {
	SINVARIANT(*i < maxi);
	SINVARIANT(found[*i] == false);
	found[*i] = true;
    }

    for(int i=0; i<maxi; ++i) {
	SINVARIANT(found[i]);
    }
}

int main()
{
    printf("test collision add/remove\n");
    
    HashTable<int,collisionHash,intEqual> collisiontable;
    for(int i=0;i<100;i++) {
	collisiontable.add(i);
    }
    for(int i=0;i<100;i+=2) {
	collisiontable.remove(i);
	size_t count = 0;
	for (HashTable<int,collisionHash,intEqual>::iterator i = collisiontable.begin();
	     i != collisiontable.end();i++) {
	    count++;
	}
	INVARIANT(count == collisiontable.size(),
		  boost::format("?! %d %d") % count % collisiontable.size());
    }
    
    int maxi = 20000;
    inttable table;
    
    printf("test adding...\n");
    for(int i=0;i<maxi;i++) {
	SINVARIANT(table.size() == static_cast<size_t>(i));
	table.add(i);
    }
    
    nonconstHTTest(table, maxi);
    constHTTest(table, maxi);

    for(int i=0;i<maxi;i++) {
	SINVARIANT(table.lookup(i) != NULL);
	SINVARIANT(table.find(i) != table.end());
	SINVARIANT(*table.lookup(i) == i);
	SINVARIANT(*table.find(i) == i);
    }
    for(inttable::iterator i = table.begin();i != table.end();i++) {
	SINVARIANT(*i >= 0 && *i < maxi);
	SINVARIANT(table.find(*i) == i);
    }
    printf("test copying...\n");
    inttable table2;
    table2 = table;
    for(int i=0;i<maxi;i++) {
	SINVARIANT(table2.lookup(i) != NULL);
	SINVARIANT(*table2.lookup(i) == i);
    }
    printf("test assigning...\n");
    inttable table3(table2);
    for(int i=0;i<maxi;i++) {
	SINVARIANT(table3.lookup(i) != NULL);
	SINVARIANT(*table3.lookup(i) == i);
    }

    printf("test removing...\n");
    for(int i=0;i<maxi/2;i++) {
	SINVARIANT(table.lookup(i) != NULL);
	SINVARIANT(*table.lookup(i) == i);
	SINVARIANT((int)table.size() == maxi-i);
	table.remove(i);
    }
    for(int i=maxi/2;i<maxi;i++) {
	SINVARIANT(table.lookup(i) != NULL);
	SINVARIANT(*table.lookup(i) == i);
    }
    for(inttable::iterator i = table.begin();i != table.end();i++) {
	SINVARIANT(*i >= maxi/2 && *i < maxi);
    }
    printf("test complete remove...\n");
    for(int i=maxi/2;i<maxi;i++) {
	SINVARIANT((int)table.size() == maxi-i);
	table.remove(i);
    }
    SINVARIANT(table.size() == 0);
    table.clear();
    SINVARIANT(table.size() == 0);
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
	    SINVARIANT(x != NULL && *x == i);
	} else {
	    SINVARIANT(x == NULL);
	    SINVARIANT(table.find(i) == table.end());
	}
    }

    table.clear();
    SINVARIANT(table.size() == 0);
    table.remove(5,false);

    stringHTTests();

    printf("success.\n");
}

