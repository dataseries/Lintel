/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file

    \brief header file for RotatingHashMap class
*/

#ifndef LINTEL_ROTATING_HASHMAP_HPP
#define LINTEL_ROTATING_HASHMAP_HPP

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <Lintel/HashMap.hpp>

// TODO: may be worth making the number of tables we keep a parameter,
// rationale is the following: Imagine you have a stream of data with
// time and some value; You want to rotate the stored data such that
// all values would be in the table for x seconds.  With two tables,
// you have to rotate every x seconds, at a cost of 2x memory.  If you
// kept 3 tables, you could rotate every x/2 seconds, because then
// even a value evicted from the first table immediately would site in
// the next to for a total of x seconds, at a cost of 3/2 x memory.
// Further tables get diminishing returns, 4 tables gets you to 4/3 x,
// and so on, and they have a linear increase in the number of tables
// that the functions have to search through.  A pity we haven't
// opened up the implementation a little more so we could (for
// example) only calculate the hash once.

/// \brief A rotating hash map
///
/// A rotating hash map.  The idea is that we want to store hash
/// entries and from time to time, flush the table of "old" values.
/// The clever optimization is to have two hash tables, and from time
/// to time, rotate the tables, deleting the old one.  If we get an
/// access to the old table, we move it to the recent one.
/// Eventually, we can have built-in rules for when to rotate can be
/// added, but for now just provide the help.
template <class K, class V,
	  class KHash = HashMap_hash<const K>,
	  class KEqual = std::equal_to<const K> >
class RotatingHashMap {
public:
    // Changing the key would destroy the hash table; changing the
    // value should be valid, although it's about to be removed from
    // the rotating hash-map.
    typedef boost::function<void (const K &, V &)> RotateFn;
    typedef HashMap<K,V,KHash,KEqual> HashMapT;

    RotatingHashMap() {
	table_recent = new HashMapT; // HashMap<K,V>;
	table_old = new HashMapT; // HashMap<K,V>;
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
		V &tmp = (*table_recent)[k] = *ret;
		table_old->remove(k);
		ret = &tmp;
	    }
	} else {
	    DEBUG_SINVARIANT(table_old->lookup(k) == NULL);
	}
	return ret;
    }

    /// Returns the value associated with the key, if it
    /// exists. Otherwise, creates an entry initialized with the
    /// default value. If it exists in the old hashmap, it moves the
    /// key-value pair to the recent hashmap.
    V &operator[](const K &k) {
	V *v = lookup(k);
	if (v == NULL) {
	    return (*table_recent)[k];
	} else {
	    return *v;
	}
    }

    /// Check to see whether a particular value exists in the rotating
    /// hash map.  This version of exists counts as an access so will
    /// promote values to the recent table.
    bool exists(const K &k) {
	return lookup(k) != NULL;
    }

    /// Check to see whether a particular value exists in the rotating
    /// hash map.  This version does not count as an access, so does
    /// not promote an old value to the recent table.
    bool existsNoPromote(const K &k) {
	V *ret = table_recent->lookup(k);
	if (ret != NULL) return true;
	return table_old->lookup(k) != NULL;
    }

    /** returns true if something was removed */
    bool remove(const K &k, bool must_exist = true) {
	bool exists_recent = table_recent->remove(k, false);
	bool exists_old = table_old->remove(k, false);
	DEBUG_SINVARIANT(!exists_recent || !exists_old);
 	SINVARIANT(!must_exist || exists_recent || exists_old);
	return exists_recent || exists_old;
    }

    void rotate() {
	delete table_old;
	table_old = table_recent;
	table_recent = new HashMapT; // HashMap<K,V>();
    }

    /** use like rotate(boost::bind(test_fn, _1, _2)); with test_fn
	taking K,V parameters, or rotate(boost::bind(&Class::fn, this,
	_1, _2)); for a class function.
	Note, it is not safe to access the RotatingHashMap that is being
	rotated in the bound function.
     */
    void rotate(const RotateFn fn) {
	for(hm_iterator i = table_old->begin();
	    i != table_old->end(); ++i) {
		fn(i->first, i->second);
	    }
	rotate();
    }
    
    /*** rotate enough times so that the hash map is empty */
    void flushRotate() {
	rotate();
	rotate();
    }

    /*** rotate enough times so that the hash map is empty */
    void flushRotate(const RotateFn fn) {
	rotate(fn);
	rotate(fn);
    }

    void walk(const RotateFn fn) {
	for(hm_iterator i = table_recent->begin(); i != table_recent->end(); ++i) {
	    fn(i->first, i->second);
	}
	for(hm_iterator i = table_old->begin(); i != table_old->end(); ++i) {
	    fn(i->first, i->second);
	}
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

    bool empty() const {
	return table_recent->empty() && table_old->empty();
    }

    /// Mostly for debugging purposes
    bool exists_recent(const K &k) {
	return table_recent->exists(k);
    }

    /// Mostly for debugging purposes
    bool exists_old(const K &k) {
	return table_old->exists(k);
    }

    size_t memoryUsage() {
	return table_recent->memoryUsage() + table_old->memoryUsage();
    }
private:
    typedef typename HashMapT::iterator hm_iterator;
    // HashMap<K,V> *table_recent, *table_old;
    HashMapT *table_recent, *table_old;
};

#endif
