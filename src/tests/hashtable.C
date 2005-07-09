/* -*-C++-*-
*******************************************************************************
*
* File:         hashtable.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/tests/hashtable.C,v 1.4 2004/04/02 22:11:09 anderse Exp $
* Description:  HashTable test program
* Author:       Eric Anderson
* Created:      Mon Jan  6 18:19:20 2003
* Modified:     Thu Mar 18 23:13:04 2004 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2003, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/
#include <stdio.h>

#include <HashTable.H>
#include <HashMap.H>
#include <ConstantString.H>

// test that hash table work done at init time is correct
ConstantString foo("foo");
ConstantString bar("bar");

HashMap<std::string,std::string> test;

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

int main()
{
    AssertAlways(strcmp(foo.c_str(),"foo")==0 &&
		 strcmp(bar.c_str(),"bar")==0,("internal error\n"));
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

    printf("success.\n");
}

