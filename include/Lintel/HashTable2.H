/* -*-C++-*- */
/*
   (c) Copyright 2002-2006, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Simple chained hash table
*/

#ifndef LINTEL_HASH_TABLE2
#define LINTEL_HASH_TABLE2

#include <boost/compressed_pair.hpp>
#include <boost/format.hpp>
#include <vector>
#include <stdint.h>

#include <AssertBoost.H>

// This is out here because C++ templates are sub-optimal.  All
// existing examples of using templates do things the way the
// following is done.  To enable use of different allocators, the
// allocator has to be a parameter of the template, but all of the
// examples only show classes as arguments to a template and an
// allocator under the C++ STL specification is a template not a
// class.

// Sample usage:
// struct hteData {
//     int key1, key2;
//     string key3;
//     string data;
//     hteData(int a, int b, const string &c, const string &d = "") 
//       : key1(a),key2(b),key3(c),data(d) {}
// };
// struct hteHash {
//     unsigned int operator()(const hteData &k) {
// 	   // use key2 as the starting hash value
//         // could also hash 2*sizeof(int) bytes starting at key1 if you are
//         // sure the values are consecutive
// 	   unsigned int partial_hash = HashTable_hashbytes(&k.key1,
// 							   sizeof(int),k.key2);
// 	   return HashTable_hashbytes(k.key3.data(),k.key3.size(),
// 				      partial_hash);
//     };
// };
// struct hteEqual {
//     bool operator()(const hteData &a, const hteData &b) {
// 	   return a.key1 == b.key1 && a.key2 == b.key2 && a.key3 == b.key3;
//     }
// };
// HashTable<hteData, hteHash, hteEqual> hashTable;
// hashTable.add(hteData(1,2,"1+2=","3"));
// hteData *result = hashTable.lookup(hteData(1,2,"1+2="));

template <class K, class V>
struct HashTable_hte {
    boost::compressed_pair<K,V> data;
    uint32_t next;
    HashTable_hte(const K &k, const V &v, uint32_t _next) : data(k, v), next(_next) {};
};

template <class K, class V, class HashFn, class Equal, 
	  class AllocHTE = std::allocator<boost::compressed_pair<K> >,
	  class AllocInt = std::allocator<uint32_t> >
class HashTable {
public:
    HashTable(double _target_chain_length) {
	init(_target_chain_length);
    }
    HashTable() {
	init(2.0);
    }
    typedef HashTable_hte<D> hte;
    typedef boost::compressed_pair<K, V> data;
    typedef std::vector<hte, AllocHTE> hte_vectorT;
    typedef std::vector<int, AllocInt> hash_tableT;

    data &add(const K &key, const D &data) {
	// The constructor is not necessarily called before the add
	// function can be called if adding to the hashtable before
	// main() (for example a static ConstantString); so we have to
	// make sure that the default initialization of all zero is
	// sufficient for this code to execute.  Note that we have
	// observed this problem on both HP-UX and Linux. For this
	// reason, the value stored in the free_chain_head_plus_one
	// variable is actually the location of a free entry + 1 as
	// the true sentry value for unused would be UINT32_MAX.
	// In fact, the chain started by free_chain_head_plus_one
	// has all the next values storing next + 1

	uint32_t hash = hashfn(key);
	uint32_t location;
	if (0 == free_chain_head_plus_one) { // No free entries
	    if (chains.size() >= target_chain_length * entry_points.size()) {
		expandHashTable();
	    }

	    location = chains.size();
	    chains.resize(chains.size() + 1);
	} else {
	    location = free_chain_head_plus_one - 1;
	    free_chain_head_plus_one = chains[location].next;
	}

	chains[location].data.first = key;
	chains[location].data.second = data;

	DEBUG_INVARIANT(entry_points.size() > 0, "internal");
	DEBUG_INVARIANT(entry_points.size() & (entry_points.size() - 1) == 0, 
			"internal not power of 2");
	hash = hash & (entry_points.size() - 1);

	DEBUG_INVARIANT(hash < entry_points.size(),("internal"));

	chains[location].next = entry_points[hash];
	entry_points[hash] = location;
	return chains[location].data;
    }

    // changing the key in the returned data will of course
    // totally screw up the hash table.
    data *lookup(const K &key) {
	if (entry_points.size() == 0) {
	    return NULL;
	}
	DEBUG_INVARIANT(entry_points.size() > 0, "internal");
	DEBUG_INVARIANT(entry_points.size() & (entry_points.size() - 1) == 0, 
			"internal not power of 2");

	uint32_t hash = hashfn(key) % entry_points.size();
	for(uint32_t i=entry_points[hash]; i != -1; i=chains[i].next) {
	    if (equal(key, chains[i].data.first)) {
		return &chains[i].data;
	    }
	}
	return NULL;
    }

    void remove(const K &key, bool must_exist = true) {
	if (entry_points.size() == 0) {
	    INVARIANT(must_exist == false,
		      boost::format("remove of %1 failed, hash table is empty") % key);
	    return;
	}
	uint32_t hash = hashfn(key) % entry_points.size();
	uint32_t loc = entry_points[hash];
	if (loc == UINT32_MAX) {
	    INVARIANT(must_exist == false,
		      boost::format("remove of %1 failed, value doesn't exist\n") % key);
	    return;
	}
	if (equal(key,chains[loc].data.key)) {
	    entry_points[hash] = chains[loc].next;
	    chains[loc].next = free_list_plus_one;
	    free_list_plus_one = loc + 1;
	} else {
	    uint32_t prev = loc;
	    while(true) {
		prev = loc;
		loc = chains[loc].next;
		if (loc == UINT32_MAX) {
		    INVARIANT(must_exist == false,
			      ("remove of %1 failed, value doesn't exist\n") % key);
		    break;
		}
		if (equal(key, chains[loc].data.key)) {
		    chains[prev].next = chains[loc].next;
		    chains[loc].next = free_list_plus_one;
		    free_list_plus_one = loc + 1;
		    break;
		}
	    }
	}
    }
	    
    void clear() {
	free_list = 0;
	chains.clear();
	for(uint32_t i=0; i<entry_points.size(); ++i) {
	    entry_points[i] = UINT32_MAX;
	}
    }

//    class iterator {
//    public:
//	iterator(HashTable &_mytable, int start_chain = 0) 
//	    : mytable(_mytable), cur_chain(start_chain), chain_loc(-1) {
//	    findNonemptyChain();
//	}
//	iterator &operator++() { increment(); return *this; }
//	iterator operator++(int) {
//	    iterator tmp = *this;
//	    increment();
//	    return tmp;
//	}
//	bool operator==(const iterator &y) const {
//	    return &mytable == &y.mytable && 
//		cur_chain == y.cur_chain && 
//		chain_loc == y.chain_loc;
//	}
//	bool operator!=(const iterator &y) const {
//	    return !(*this == y);
//	}
//	D &operator *() { 
//	    AssertAlways(chain_loc >= 0 && 
//			 chain_loc < (int)mytable.chains.size(),
//			 ("Bad use of iterator\n"));
//	    return mytable.chains[chain_loc].data;
//	}
//	D *operator ->() {
//	    return &(operator *());
//	}	    
//	// use this while iterating over an entire hash table, but you
//	// don't want to do the entire scan as a single operation (for
//	// example for time reasons), this operation allows you to
//	// restart the scan operation partway through after doing some
//	// number of updates safely.
//	void partialReset() {
//	    if (cur_chain < (int)mytable.entry_points.size()) {
//		chain_loc = mytable.entry_points[cur_chain];
//		findNonemptyChain();
//	    } else {
//		AssertAlways(cur_chain == (int)mytable.entry_points.size(),
//			     ("internal error\n"));
//	    }
//	}
//	void reset() {
//	    cur_chain = 0;
//	    findNonemptyChain();
//	}
//	int getCurChain() {
//	    return cur_chain;
//	}
//    private:
//	void findNonemptyChain() {
//	    while(cur_chain < (int)mytable.entry_points.size() &&
//		  mytable.entry_points[cur_chain] == -1) {
//		cur_chain += 1;
//	    }
//	    if (cur_chain < (int)mytable.entry_points.size()) {
//		chain_loc = mytable.entry_points[cur_chain];
//	    }
//	}
//	void increment() {
//	    AssertAlways(chain_loc >= 0 && 
//			 chain_loc < (int)mytable.chains.size(),
//			 ("bad use of iterator\n"));
//	    chain_loc = mytable.chains[chain_loc].next;
//	    if (chain_loc == -1) {
//		cur_chain += 1;
//		findNonemptyChain();
//	    }
//	}
//	HashTable &mytable;
//	int cur_chain;
//	int chain_loc;
//    };
//    
//    iterator begin(int start_chain = 0) {
//	return iterator(*this,start_chain);
//    }
//    iterator end() {
//	return iterator(*this,entry_points.size());
//    }
//    
//    int size() {
//	int size = chains.size();
//	for(int i = free_list; i != -1; i = chains[i].next) {
//	    size--;
//	}
//	return size;
//    }
//
//    HashTable(const HashTable &__in) {
//	assign(__in);
//    }
//
//    HashTable &
//    operator=(const HashTable &__in) {
//	return assign(__in);
//    }
//    
//    void reserve(unsigned expected_entries) {
//	// assertion lets us resize the entry points without thinking about
//	// handling existing bits.
//	AssertAlways(chains.size() == 0,
//		     ("have to reserve() before putting anything in hash table"));
//	chains.reserve(expected_entries);
//	unsigned new_entry_size = (unsigned)(expected_entries / target_chain_length);
//	unsigned i;
//	for(i=0; HashTable_prime_list[i] < new_entry_size; ++i) {
//	    AssertAlways(HashTable_prime_list[i] != 0,("Fatal Error"));
//	}
//	unsigned new_size = HashTable_prime_list[i];
//
//	entry_points.resize(new_size, -1);
//    }
//
//    // this is useful if you want to build up a big table of data, but then
//    // want to store it on disk sorted, of course once you sort the vector,
//    // you've just destroyed the hash table, but the only other choice
//    // is to hash pointers to the data, which loses more memory space.
//
//    hte_vectorT &unsafeGetRawDataVector() {
//	return chains;
//    }
private:
    void init(double _target_chain_length) {
	free_list_plus_one = 0;
	target_chain_length = _target_chain_length;
	INVARIANT(target_chain_length > 0,
		  boost::format("invalid target_chain_length %.6f\n")
		  % target_chain_length);
    }	

//    HashTable &assign(const HashTable &__in) {
//	free_list = __in.free_list;
//	chains = __in.chains;
//	entry_points = __in.entry_points;
//	target_chain_length = __in.target_chain_length;
//	hashfn = __in.hashfn;
//	equal = __in.equal;
//	return *this;
//    }	
    
//     friend class iterator;
    // Still keep hash table with prime size; this helps hash functions
    // which are mediocre, even though the hash function at the end
    // of the header is good enough that factor of 2 hash table sizes
    // are fine.

    void expandHashTable() {
	INVARIANT(free_list == 0,"?! internal error, expanding with free values still present??\n");
	uint32_t old_size = entry_points.size();
	uint32_t new_size;


	if (0 == old_size) { 
	    new_size = 8;
	}
	entry_points.reserve(new_size);
	entry_points.resize(new_size,-1);

	// Clear old entries & links
	for(i=0;i<old_size;i++) {
	    entry_points[i] = -1;
	}
	for(i=0;i<chains.size();i++) {
	    chains[i].next = -1;
	}

	for(i=0;i<chains.size();i++) {
	    int hash = hashfn(chains[i].data) % new_size;
	    chains[i].next = entry_points[hash];
	    entry_points[hash] = i;
	}
    }
    
    int free_list; // for chains
    hte_vectorT chains;
    hash_tableT entry_points;
    double target_chain_length;
    HashFn hashfn;
    Equal equal;
};

// prev_hash allows you to hash data where the key is in separate pieces
// it may not get the same hash as concatenating the key would give
// 1972 is an arbitrary start

// note that the order of arguments here is different than for the
// HashTable_hashbytes function; this is to make the order consistent
// with how the crc/adler hash's work in both zlib and lzo library, and
// the ordering for the md5, sha1, etc. hashs from openssl; the order for
// the other function is retained as the common usage is to not specify a
// previous hash


uint32_t BobJenkinsHash(const uint32_t prev_hash,
			    const void *bytes, const uint32_t size);
inline uint32_t HashTable_hashbytes(const void *bytes, 
					const uint32_t size,
					const uint32_t prev_hash = 1972)
{ 
    return BobJenkinsHash(prev_hash, bytes, size);
}

// A fast way of doing an inline mix of three integers; used in the
// BobJenkins Hash as a core operation; this is placed here so that
// hashing on a bunch of small integers can be done quickly without a
// function call and all of the variable length overhead inherent in
// the general purpose hash.

#define BobJenkinsHashMix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

static inline uint32_t BobJenkinsHashMix3(uint32_t a, uint32_t b, uint32_t c)
{
    BobJenkinsHashMix(a,b,c);
    return c;
}

static inline uint32_t BobJenkinsHashMixULL(unsigned long long v)
{
    uint32_t a = static_cast<uint32_t>(v & 0xFFFFFFFF);
    uint32_t b = static_cast<uint32_t>((v >> 32) & 0xFFFFFFFF);
    uint32_t c = 1972;
    BobJenkinsHashMix(a,b,c);
    return c;
}

#endif
#include <Lintel/HashTable2.hpp>
