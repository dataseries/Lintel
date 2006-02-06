/* -*-C++-*-
/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Simple chained hash table
*/

#ifndef __HASH_TABLE
#define __HASH_TABLE

#include <vector>

#include <LintelAssert.H>
extern unsigned int HashTable_prime_list[];

// The key and value are not separate because in some cases, they are
// the same, for example for ConstantString, which wants to do a lookup to
// find a constant string

// HashFn should only hash the key part of D; similarly Equal should only
// compare on the key part of D

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

template <class D>
struct HashTable_hte {
    D data;
    int next;
    HashTable_hte(const D &d, int _next) : data(d), next(_next) {};
};

template <class D, class HashFn, class Equal, 
#ifdef RWSTD_SIMPLE_DEFAULT
    class AllocHTE RWSTD_SIMPLE_DEFAULT(allocator),
    class AllocInt RWSTD_SIMPLE_DEFAULT(allocator) >
#else
    class AllocHTE = std::allocator<HashTable_hte<D> >,
    class AllocInt = std::allocator<int> >
#endif
class HashTable {
public:
    HashTable(double _target_chain_length) {
	init(_target_chain_length);
    }
    HashTable() {
	init(2.0);
    }
    typedef HashTable_hte<D> hte;
    typedef std::vector<hte, AllocHTE> hte_vectorT;
    typedef std::vector<int, AllocInt> hash_tableT;
public:
    D *add(const D &data) {
	// The constructor is not necessarily called before the add
	// function can be called if adding to the hashtable before
	// main() (for example a static ConstantString); so force
	// initialization in this case.  Recently found a case where
	// this can happen on Linux as well, so do this in general
	if (entry_points.capacity() == 0) {
	    AssertAlways(free_list == 0 && target_chain_length == 0,
			 ("weird vector behavior %p %d %.10f",
			  this,free_list,target_chain_length));
	    free_list = -1;
	    target_chain_length = 2.0;

	    chains.reserve(1); // needed on HP-UX, doesn't hurt on Linux
	}
	if (free_list == -1 && 
	    chains.size() >= target_chain_length * entry_points.size()) {
	    resizeHashTable();
	}
#ifdef COMPILE_DEBUG
	AssertAlways(entry_points.size() > 0,("internal"));
#endif
	int hash = hashfn(data) % entry_points.size();
	if (free_list == -1) {
	    hte v(data, entry_points[hash]);
#ifdef COMPILE_DEBUG
	    AssertAlways(hash < (int)entry_points.size(),("internal"));
#endif
	    entry_points[hash] = (int)chains.size();
	    chains.push_back(v);
	    return &(chains.back().data);
	} else {
	    int loc = free_list;
#ifdef COMPILE_DEBUG
	    AssertAlways(loc < (int)chains.size(),("internal"));
#endif
	    free_list = chains[free_list].next;
	    chains[loc].data = data;
	    chains[loc].next = entry_points[hash];
	    entry_points[hash] = loc;
	    return &chains[loc].data;
	}
    }
    // changing the key in the returned key/value data will of course
    // totally screw up the hash table.
    D *lookup(const D &key) {
	if (entry_points.size() == 0) {
	    return NULL;
	}
	int hash = hashfn(key) % entry_points.size();
	for(int i=entry_points[hash];i != -1; i=chains[i].next) {
	    if (equal(key,chains[i].data)) {
		return &chains[i].data;
	    }
	}
	return NULL;
    }

    void remove(const D &key, bool must_exist = true) {
	if (entry_points.size() == 0) {
	    AssertAlways(must_exist == false,("remove failed, hash table is empty"));
	    return;
	}
	int hash = hashfn(key) % entry_points.size();
	int loc = entry_points[hash];
	if (loc == -1) {
	    AssertAlways(must_exist == false,
			 ("remove failed, value doesn't exist\n"));
	    return;
	}
	if (equal(key,chains[loc].data)) {
	    entry_points[hash] = chains[loc].next;
	    chains[loc].next = free_list;
	    free_list = loc;
	} else {
	    int prev = loc;
	    while(true) {
		prev = loc;
		loc = chains[loc].next;
		if (loc == -1) {
		    AssertAlways(must_exist == false,
				 ("remove failed, value doesn't exist\n"));
		    break;
		}
		if (equal(key,chains[loc].data)) {
		    chains[prev].next = chains[loc].next;
		    chains[loc].next = free_list;
		    free_list = loc;
		    break;
		}
	    }
	}
    }
	    
    void clear() {
	free_list = -1;
	chains.clear();
	for(unsigned int i=0;i<entry_points.size();i++) {
	    entry_points[i] = -1;
	}
    }

    class iterator {
    public:
	iterator(HashTable &_mytable, int start_chain = 0) 
	    : mytable(_mytable), cur_chain(start_chain), chain_loc(-1) {
	    findNonemptyChain();
	}
	iterator &operator++() { increment(); return *this; }
	iterator operator++(int) {
	    iterator tmp = *this;
	    increment();
	    return tmp;
	}
	bool operator==(const iterator &y) const {
	    return &mytable == &y.mytable && 
		cur_chain == y.cur_chain && 
		chain_loc == y.chain_loc;
	}
	bool operator!=(const iterator &y) const {
	    return !(*this == y);
	}
	D &operator *() { 
	    AssertAlways(chain_loc >= 0 && 
			 chain_loc < (int)mytable.chains.size(),
			 ("Bad use of iterator\n"));
	    return mytable.chains[chain_loc].data;
	}
	D *operator ->() {
	    return &(operator *());
	}	    
	// use this while iterating over an entire hash table, but you
	// don't want to do the entire scan as a single operation (for
	// example for time reasons), this operation allows you to
	// restart the scan operation partway through after doing some
	// number of updates safely.
	void partialReset() {
	    if (cur_chain < (int)mytable.entry_points.size()) {
		chain_loc = mytable.entry_points[cur_chain];
		findNonemptyChain();
	    } else {
		AssertAlways(cur_chain == (int)mytable.entry_points.size(),
			     ("internal error\n"));
	    }
	}
	void reset() {
	    cur_chain = 0;
	    findNonemptyChain();
	}
	int getCurChain() {
	    return cur_chain;
	}
    private:
	void findNonemptyChain() {
	    while(cur_chain < (int)mytable.entry_points.size() &&
		  mytable.entry_points[cur_chain] == -1) {
		cur_chain += 1;
	    }
	    if (cur_chain < (int)mytable.entry_points.size()) {
		chain_loc = mytable.entry_points[cur_chain];
	    }
	}
	void increment() {
	    AssertAlways(chain_loc >= 0 && 
			 chain_loc < (int)mytable.chains.size(),
			 ("bad use of iterator\n"));
	    chain_loc = mytable.chains[chain_loc].next;
	    if (chain_loc == -1) {
		cur_chain += 1;
		findNonemptyChain();
	    }
	}
	HashTable &mytable;
	int cur_chain;
	int chain_loc;
    };
    
    iterator begin(int start_chain = 0) {
	return iterator(*this,start_chain);
    }
    iterator end() {
	return iterator(*this,entry_points.size());
    }
    
    int size() {
	int size = chains.size();
	for(int i = free_list; i != -1; i = chains[i].next) {
	    size--;
	}
	return size;
    }

    HashTable(const HashTable &__in) {
	assign(__in);
    }

    HashTable &
    operator=(const HashTable &__in) {
	return assign(__in);
    }
    
private:
    void init(double _tcl) {
	free_list = -1;
	target_chain_length = _tcl;
	AssertAlways(target_chain_length > 0,
		     ("invalid target_chain_length %.6f\n",
		      target_chain_length));
	entry_points.reserve(1); 
	// make the below "have we initted" call in add() go as fast
	// as possible
    }	
    HashTable &assign(const HashTable &__in) {
	free_list = __in.free_list;
	chains = __in.chains;
	entry_points = __in.entry_points;
	target_chain_length = __in.target_chain_length;
	hashfn = __in.hashfn;
	equal = __in.equal;
	return *this;
    }	
    
    friend class iterator;
    // Still keep hash table with prime size; this helps hash functions
    // which are mediocre, even though the hash function at the end
    // of the header is good enough that factor of 2 hash table sizes
    // are fine.

    void resizeHashTable() {
	AssertAlways(free_list == -1,("?! internal error\n"));
	unsigned int old_size = entry_points.size();
	unsigned int new_size;
	unsigned int i;
	for(i=0; HashTable_prime_list[i] <= old_size;i++) {
	    AssertAlways(HashTable_prime_list[i] != 0,("Fatal Error"));
	}
	new_size = HashTable_prime_list[i];
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


unsigned int BobJenkinsHash(const unsigned int prev_hash,
			    const void *bytes, const unsigned int size);
inline unsigned int HashTable_hashbytes(const void *bytes, 
					const unsigned int size,
					const unsigned int prev_hash = 1972)
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

static inline unsigned int BobJenkinsHashMix3(unsigned int a, unsigned int b, unsigned int c)
{
    BobJenkinsHashMix(a,b,c);
    return c;
}

static inline unsigned int BobJenkinsHashMixULL(unsigned long long v)
{
    unsigned int a = (unsigned int)(v & 0xFFFFFFFF);
    unsigned int b = (unsigned int)((v >> 32) & 0xFFFFFFFF);
    unsigned int c = 1972;
    BobJenkinsHashMix(a,b,c);
    return c;
}

#endif
