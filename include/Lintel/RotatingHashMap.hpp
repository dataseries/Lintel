/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file

    A rotating hash map.  The idea is that we want to store hash
    entries and from time to time, flush the table of "old" values.
    The clever optimization is to have two hash tables, and from time
    to time, rotate the tables, deleting the old one.  If we get an
    access to the old table, we move it to the recent one.
    Eventually, we can have built-in rules for when to rotate can be
    added, but for now just provide the help.
*/

#ifndef __LINTEL_ROTATING_HASHMAP_HPP
#define __LINTEL_ROTATING_HASHMAP_HPP

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <Lintel/HashMap.H>

template <class K, class V,
	  class KHash = HashMap_hash<const K>,
	  class KEqual = std::equal_to<const K> >
class RotatingHashMap {
public:
    typedef boost::function<void (const K &, const V &)> rotate_fn;
    typedef HashMap<K,V> HashMapT;

    RotatingHashMap() {
	table_recent = new HashMap<K,V>;
	table_old = new HashMap<K,V>;
    }

    ~RotatingHashMap() {
	delete table_recent;
	delete table_old;
    }

    V *lookup(const K &k) {
	V *ret = table_recent->lookup(k);
	if (ret == NULL) {
	    ret = table_old->lookup(k);
	    if (ret != NULL) {
		(*table_recent)[k] = *ret;
		table_old->remove(k);
	    }
	} else {
	    DEBUG_SINVARIANT(table_old->lookup(k) == NULL);
	}
	return ret;
    }

    V &operator[](const K &k) {
	V *v = lookup(k);
	if (v == NULL) {
	    return (*table_recent)[k];
	} else {
	    return *v;
	}
    }

    /* Could want to make this take an argument so an exists check
       does not count as a use for promoting to recent */
    bool exists(const K &k) {
	return lookup(k) != NULL;
    }

    /** returns true if something was removed */
    bool remove(const K &k, bool must_exist = true) {
	bool exists_recent = table_recent->remove(k, false);
	bool exists_old = table_old->remove(k, false);
	SINVARIANT(exists_recent ^ exists_old);
	SINVARIANT(!must_exist || exists_recent || exists_old);
	return exists_recent || exists_old;
    }

    void rotate() {
	delete table_old;
	table_old = table_recent;
	table_recent = new HashMap<K,V>();
    }

    /** use like rotate(boost::bind(test_fn, _1, _2)); with test_fn
	taking K,V parameters, or rotate(boost::bind(&Class::fn, this,
	_1, _2)); for a class function.
	Note, it is not safe to access the RotatingHashMap that is being
	rotated in the bound function.
     */
    void rotate(const rotate_fn fn) {
	for(hm_iterator i = table_old->begin();
	    i != table_old->end(); ++i) {
		fn(i->first, i->second);
	    }
	rotate();
    }
    
    size_t size_recent() {
	return table_recent->size();
    }

    size_t size_old() {
	return table_old->size();
    }

    size_t size() {
	return size_recent() + size_old();
    }
    
private:
    typedef typename HashMapT::iterator hm_iterator;
    HashMap<K,V> *table_recent, *table_old;
};

#endif
