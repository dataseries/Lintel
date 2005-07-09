/* -*-C++-*-
*******************************************************************************
*
* File:         HashUnique.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/HashUnique.H,v 1.1 2004/04/02 22:11:08 anderse Exp $
* Description:  hash implementation of uniqueifying a set of values
* Author:       Eric Anderson
* Created:      Wed Mar 24 10:59:42 2004
* Modified:     Wed Mar 24 12:24:25 2004 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2004, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef __HASH_UNIQUE_H
#define __HASH_UNIQUE_H

#include <HashMap.H>

template <class K,
          class KHash = HashMap_hash<const K>,
          class KEqualFn = std::equal_to<const K> >
class HashUnique {
public:
    struct KEqual {
	KEqualFn fn;
	bool operator()(const K &a, const K &b) {
	    return fn(a,b);
	}
    };
    
    bool exists(const K &k) {
	K *v = hashtable.lookup(k);
	return v != NULL;
    }
    
    void add(const K &k) {
	K *v = hashtable.lookup(k);
	if (v == NULL) {
	    hashtable.add(k);
	}
    }
    
    void remove(const K &k) {
	hashtable.remove(k);
    }

    typedef HashTable<K, KHash, KEqual> HashTableT;

    typedef typename HashTableT::iterator iterator;

    iterator begin() {
	return hashtable.begin();
    }
    
    iterator end() {
	return hashtable.end();
    }

    HashUnique() {
    }

    HashUnique(const HashUnique &__in) {
	hashtable = __in.hashtable;
    }

    HashUnique &
    operator=(const HashUnique &__in) {
	hashtable = __in.hashtable;
	return *this;
    }
private:
    HashTableT hashtable;
};

#endif
