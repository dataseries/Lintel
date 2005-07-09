/* -*-C++-*-
*******************************************************************************
*
* File:         HashMap.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/HashMap.H,v 1.4 2004/09/28 01:20:18 anderse Exp $
* Description:  A "map" using the HashTable class
* Author:       Eric Anderson
* Created:      Wed Nov  5 09:59:30 2003
* Modified:     Sun Sep 26 22:41:34 2004 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2003, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef __HASH_MAP
#define __HASH_MAP

#if __GNUG__ == 3 && __GNUC_MINOR == 3
#include <ext/stl_hash_fun.h>
#endif
#if __GNUG__ == 3 && __GNUC_MINOR == 4
#include <ext/hash_fun.h>
#endif
#include <functional>

#include <HashTable.H>

template <class K, class V>
struct HashMap_val {
    K first;
    V second;
    HashMap_val(K &k, V &v) : first(k), second(v) { }
    HashMap_val(const K &k) : first(k) { }
};

template <class K>
struct HashMap_hash {
    //    unsigned operator()(const K &a) const;
};

template <>
struct HashMap_hash<const std::string> {
    unsigned operator()(const std::string &a) const {
	return HashTable_hashbytes(a.data(),a.size());
    }
};

template <>
struct HashMap_hash<const int> {
    unsigned operator()(const int _a) const {
	// doesn't increase the entropy, but shuffles the entropy across
	// the entire integer, meaning the low bits get shuffled even if they
	// are constant for some reason
	int a = _a;
	int b = 0xBEEFCAFE;
	int ret = 1972;
	BobJenkinsHashMix(a,b,ret);
	return ret;
    }
};

template <class K, class V, 
          class KHash = HashMap_hash<const K>, 
          class KEqual = std::equal_to<const K> >
class HashMap {
public:
    typedef HashMap_val<K,V> hmval;
    struct hmvalHash {
	KHash khash;
	unsigned int operator()(const hmval &hmv) {
	    return khash(hmv.first);
	}
    };
    struct hmvalEqual {
	KEqual kequal;
	bool operator()(const hmval &a, const hmval &b) {
	    return kequal(a.first,b.first);
	}
    };

    V *lookup(const K &k) {
	hmval fullval(k);
	hmval *v = hashtable.lookup(fullval);
	if (v == NULL) {
	    return NULL;
	} else {
	    return &v->second;
	}
    }

    V &operator[] (const K &k) {
	hmval fullval(k);
	hmval *v = hashtable.lookup(fullval);
	if (v == NULL) {
	    return hashtable.add(fullval)->second;
	} else {
	    return v->second;
	}
    }

    V &operator[] (K &k) {
	hmval fullval(k);
	hmval *v = hashtable.lookup(fullval);
	if (v == NULL) {
	    return hashtable.add(fullval)->second;
	} else {
	    return v->second;
	}
    }

    void remove(K &k) {
	hmval fullval(k);
	hashtable.remove(fullval);
    }

    int size() {
	return hashtable.size();
    }

    typedef typename HashTable<hmval,hmvalHash,hmvalEqual>::iterator iterator;

    iterator begin() {
	return hashtable.begin();
    }
    iterator end() {
	return hashtable.end();
    }

    HashMap() {
    }

    HashMap(const HashMap &__in) {
	hashtable = __in.hashtable;
    }

    HashMap &
    operator=(const HashMap &__in) {
	hashtable = __in.hashtable;
	return *this;
    }
private:
    HashTable<hmval,hmvalHash,hmvalEqual> hashtable;
};

#endif
