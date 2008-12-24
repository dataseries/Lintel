/*
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Simple chained hash table
*/

#ifndef LINTEL_HASH_TABLE_HPP
#define LINTEL_HASH_TABLE_HPP

#include <stdint.h>

#include <iterator>
#include <vector>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/CompilerMarkup.hpp>
#include <Lintel/HashFns.hpp>
#include <Lintel/Stats.hpp>

// these used to be in the global namespace; preserve that for now.
// TODO-2009-06-01: remove deprecated functions
FUNC_DEPRECATED_PREFIX inline uint32_t 
BobJenkinsHash(const uint32_t prev_hash, const void *bytes, const size_t size) FUNC_DEPRECATED;

inline uint32_t BobJenkinsHash(const uint32_t prev_hash,
			       const void *bytes, const size_t size) {
    return lintel::bobJenkinsHash(prev_hash, bytes, size);
}
FUNC_DEPRECATED_PREFIX inline uint32_t 
HashTable_hashbytes(const void *bytes, const size_t size, 
		    const uint32_t prev_hash) FUNC_DEPRECATED;

inline uint32_t HashTable_hashbytes(const void *bytes, const size_t size,
				    const uint32_t prev_hash = 1972) { 
    return lintel::hashBytes(bytes, size, prev_hash);
}
using lintel::BobJenkinsHashMix3;
using lintel::BobJenkinsHashMixULL;
#define BobJenkinsHashMix lintel_BobJenkinsHashMix

extern uint32_t HashTable_prime_list[];

/// This is out here because C++ templates are sub-optimal.  All
/// existing examples of using templates do things the way the
/// following is done.  To enable use of different allocators, the
/// allocator has to be a parameter of the template, but all of the
/// examples only show classes as arguments to a template and an
/// allocator under the C++ STL specification is a template not a
/// class.
template <class D>
struct HashTable_hte {
    D data;
    int32_t next;
    HashTable_hte(const D &d, int32_t _next) : data(d), next(_next) {};
};

// TODO: fix this so that instead of using -1, it uses 0xFFFFFFFF as
// the sentinal value, then make sure the use of uint32_t is proper
// through all of this so we can bump the maximum hash table size to
// 2^32-1 entries.  Consider writing a test (64bit only) that can
// verify proper operation at that size.

/// The HashTable class is the low-level class for working with
/// hash-tables.  For most uses you probably want to just use HashMap,
/// or HashUnique.
///
/// \code
/// Sample usage:
/// struct hteData {
///     int key1, key2;
///     string key3;
///     string data;
///     hteData(int a, int b, const string &c, const string &d = "") 
///       : key1(a),key2(b),key3(c),data(d) {}
/// };
/// struct hteHash {
///     uint32_t operator()(const hteData &k) {
/// 	   // use key1+key2 as the starting hash value; 1972 is an arbitrary
///        // constant. Could use HashTable_hashbytes, but that would be 
///        // slower and would end up calculating about the same thing.
///        uint32_t partial_hash = BobJenkinsHashMix3(k.key1, key.key2, 1972);
///        // now mix in the string key value.
/// 	   return HashTable_hashbytes(k.key3.data(),k.key3.size(),
/// 				      partial_hash);
///     };
/// };
/// struct hteEqual {
///     bool operator()(const hteData &a, const hteData &b) {
/// 	   return a.key1 == b.key1 && a.key2 == b.key2 && a.key3 == b.key3;
///     }
/// };
/// HashTable<hteData, hteHash, hteEqual> hashTable;
/// hashTable.add(hteData(1,2,"1+2=","3"));
/// hteData *result = hashTable.lookup(hteData(1,2,"1+2="));
/// \endcode
///
/// WARNING: if you put a pointer into the hteData, or the value of a
/// HashMap, you can't safely put a delete for the pointer into the
/// destructor for the structure.  The problem is that during
/// operation, the data structure may be copied, and the old version
/// deleted.  This destroys the pointed to value but leaves around the
/// pointer.  An stl::map behaves the same way.  
///
/// The key and value are not separate because in some cases, they are
/// the same, for example for ConstantString, which wants to do a lookup to
/// find a constant string.
///
/// HashFn should only hash the key part of D; similarly Equal should only
/// compare on the key part of D.


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
    explicit HashTable(double _target_chain_length) {
	init(_target_chain_length);
    }
    HashTable() {
	init(2.0);
    }
    typedef HashTable_hte<D> hte;
    typedef std::vector<hte, AllocHTE> hte_vectorT;
    typedef std::vector<int, AllocInt> hash_tableT;
public:
    /// Add in a new entry to the hash table; will always add in the
    /// value even if there is an existing value v that is Equal(data,
    /// v).
    D *add(const D &data) {
	uint32_t hashof_ = hashof(data);
	return internal_add(data, hashof_);
    }

    /// Add in a new entry to the hash table if the key does not
    /// exist.  If the key does exist, replace the existing data.
    D *addOrReplace(const D &data, bool &replaced) {
	uint32_t hashof_ = hashof(data);
	hte *chain = internal_lookup(data, hashof_);
	if (chain != NULL) {
	    replaced = true;
	    return &(chain->data);
	} else {
	    replaced = false;
	    return internal_add(data, hashof_);
	}
    }

    // changing the key in the returned key/value data will of course
    // totally screw up the hash table.
    D *lookup(const D &data) {
	uint32_t hashof_ = hashof(data);
	hte *chain = internal_lookup(data, hashof_);
	if (chain != NULL) {
	    return &(chain->data);
	} else {
	    return NULL;
	}
    }

    /// const version of lookup, hence returns constant data.
    const D *lookup(const D &data) const {
	uint32_t hashof_ = hashof(data);
	const hte *chain = internal_lookup(data, hashof_);
	if (chain != NULL) {
	    return &(chain->data);
	} else {
	    return NULL;
	}
    }

    // Not perfectly random, if there are biases in the hash(key)
    // data, then some chains will be much longer than others and so
    // will be less likely to be chosen.  Unfortunately, a true random
    // lookup would be O(size) because it would have to walk all of
    // the chains to count the number of entries.
    // 
    // Warning: the order of evaluation for arguments to a function
    // is not fixed.  This means that if you call randomlookup(rng(), rng())
    // the values of randa and randb may be flipped in different
    // compilations.
    D *randomlookup(const uint32_t randa, const uint32_t randb) {
	if (size() == 0) {
	    return NULL;
	}
	uint32_t start_chain = randa % entry_points.size();
	uint32_t chain = start_chain;
	while(entry_points[chain] == -1) {
	    chain = (chain + 1) % entry_points.size();
	    INVARIANT(chain != start_chain, 
		      "unable to find a non-empty chain");
	}

	uint32_t chain_len = 0;
	for(int32_t j = entry_points[chain]; j != -1; j=chains[j].next) {
	    ++chain_len;
	}

	SINVARIANT(chain_len > 0);

	uint32_t bucket = randb % chain_len;
	for(int32_t j = entry_points[chain]; j != -1; j=chains[j].next) {
	    if (bucket == 0) {
		return &chains[j].data;
	    } else {
		--bucket;
	    }
	}
	FATAL_ERROR("internal: did not find random entry");
	return NULL;
    }

    /* return true if something was found to remove */
    bool remove(const D &key, bool must_exist = true) {
	if (entry_points.size() == 0) {
	    INVARIANT(must_exist == false,
		      "remove failed, hash table is empty");
	    return false;
	}
	uint32_t hash = hashof(key) % entry_points.size();
	int32_t loc = entry_points[hash];
	if (loc == -1) {
	    INVARIANT(must_exist == false,
		      "remove failed, value doesn't exist");
	    return false;
	}

	bool ret = true;
	if (equal(key,chains[loc].data)) {
	    chains[loc].data = D();
	    entry_points[hash] = chains[loc].next;
	    chains[loc].next = free_list;
	    free_list = loc;
	} else {
	    int32_t prev = loc;
	    while(true) {
		prev = loc;
		loc = chains[loc].next;
		if (loc == -1) {
		    INVARIANT(must_exist == false,
			      "remove failed, value doesn't exist");
		    ret = false;
		    break;
		}
		if (equal(key,chains[loc].data)) {
		    chains[loc].data = D();
		    chains[prev].next = chains[loc].next;
		    chains[loc].next = free_list;
		    free_list = loc;
		    break;
		}
	    }
	}
	return ret;
    }

     void clear() {
	 free_list = -1;
	 chains.clear();
	 for(uint32_t i=0;i<entry_points.size();i++) {
	     entry_points[i] = -1;
	 }
     }

    /// Get statistics for the chain lengths of all the chains in a
    /// hash table.  Useful for detecting a bad hash function.
    void chainLengthStats(Stats &stats) const {
	 for(uint32_t i=0;i<entry_points.size();i++) {
	     uint32_t len = 0;
	     for(int32_t j = entry_points[i]; j != -1; j = chains[j].next) {
		 ++len;
	     }
	     stats.add(len);
	 }
    }

private:
    template<typename t_value_type, class t_hashtable_type>
    class iterator_base {
    public:
	typedef class iterator_base<t_value_type, t_hashtable_type> SelfT;
	typedef std::forward_iterator_tag iterator_category;
	typedef t_value_type value_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef std::ptrdiff_t difference_type;

	bool operator==(const SelfT &y) const {
	    return this->mytable == y.mytable 
 	        && this->cur_chain == y.cur_chain 
	        && this->chain_loc == y.chain_loc;
	}

	bool operator!=(const SelfT &y) const {
	    return !(*this == y);
	}

	t_value_type &operator *() { 
	    INVARIANT(this->chain_loc >= 0 && 
		      this->chain_loc < static_cast<int32_t>(this->mytable->chains.size()),
		      "Bad use of iterator");
	    return this->mytable->chains[this->chain_loc].data;
	}

	t_value_type *operator ->() {
	    return &(operator *());
	}	    

	/// use this while iterating over an entire hash table, but you
	/// don't want to do the entire scan as a single operation (for
	/// example for time reasons), this operation allows you to
	/// restart the scan operation partway through after doing some
	/// number of updates safely.
	void partialReset() {
	    if (cur_chain < static_cast<int>(mytable->entry_points.size())) {
		chain_loc = mytable->entry_points[cur_chain];
		findNonemptyChain();
	    } else {
		SINVARIANT(cur_chain == static_cast<int>(mytable->entry_points.size()));
	    }
	}
	void reset() {
	    cur_chain = 0;
	    findNonemptyChain();
	}
	int32_t getCurChain() {
	    return cur_chain;
	}
    protected:
	iterator_base(t_hashtable_type *_mytable, int32_t start_chain = 0,
		      int32_t _chain_loc = -1) 
	    : mytable(_mytable), cur_chain(start_chain), 
	      chain_loc(_chain_loc) {
		  if (chain_loc == -1) {
		      findNonemptyChain();
		  }
	    }
	void findNonemptyChain() {
	    while(cur_chain < static_cast<int>(mytable->entry_points.size()) &&
		  mytable->entry_points[cur_chain] == -1) {
		      cur_chain += 1;
		  }
	    if (cur_chain < static_cast<int>(mytable->entry_points.size())) {
		chain_loc = mytable->entry_points[cur_chain];
	    }
	}
	void increment() {
	    INVARIANT(chain_loc >= 0 && chain_loc < static_cast<int>(mytable->chains.size()),
		      boost::format("bad use of iterator %d not in [0,%d[") % chain_loc
		      % mytable->chains.size());
	    chain_loc = mytable->chains[chain_loc].next;
	    if (chain_loc == -1) {
		cur_chain += 1;
		findNonemptyChain();
	    }
	}
	t_hashtable_type *mytable;
	int32_t cur_chain;
	int32_t chain_loc;
    };
public:
    // TODO: finish making this satisfy all the bits from
    // /usr/include/boost/concept_archetype
    class iterator : public iterator_base<D, HashTable> {
    public:
	iterator(const iterator &from) 
	    : iterator_base<D, HashTable>
  	          (from.mytable, from.cur_chain, from.chain_loc) { }
	iterator(HashTable &mytable, int32_t start_chain = 0, 
		 int32_t chain_loc = -1)
	    : iterator_base<D, HashTable>(&mytable, start_chain, chain_loc) { }
	
	iterator &operator++() { this->increment(); return *this; }
	iterator operator++(int) {
	    iterator tmp = *this;
	    this->increment();
	    return tmp;
	}
	iterator &operator =(const iterator &right) {
	    this->mytable = right.mytable;
	    this->cur_chain = right.cur_chain;
	    this->chain_loc = right.chain_loc;
	    return *this;
	}
    };
    
    iterator begin(int32_t start_chain = 0) {
	return iterator(*this,start_chain);
    }

    iterator end() {
	return iterator(*this,entry_points.size());
    }

    iterator find(const D &key) {
	if (entry_points.size() == 0) {
	    return end();
	}
	uint32_t hash = hashof(key) % entry_points.size();
	for(int32_t i=entry_points[hash];i != -1; i=chains[i].next) {
	    if (equal(key,chains[i].data)) {
		return iterator(*this, hash, i);
	    }
	}
	return end();
    }

    class const_iterator : public iterator_base<const D, const HashTable> {
    public:
	const_iterator(const HashTable &mytable, int32_t start_chain = 0,
		       int32_t chain_loc = -1)
	    : iterator_base<const D, const HashTable>
	          (&mytable, start_chain, chain_loc) 
	{ }
	
	const_iterator &operator++() { this->increment(); return *this; }
	const_iterator operator++(int) {
	    const_iterator tmp = *this;
	    this->increment();
	    return tmp;
	}
    };

    const_iterator begin(int32_t start_chain = 0) const {
	return const_iterator(*this, start_chain);
    }

    const_iterator end() const {
	return const_iterator(*this, entry_points.size());
    }

    const_iterator find(const D &key) const {
	if (entry_points.size() == 0) {
	    return end();
	}
	uint32_t hash = hashof(key) % entry_points.size();
	for(int32_t i=entry_points[hash];i != -1; i=chains[i].next) {
	    if (equal(key, chains[i].data)) {
		return const_iterator(*this, hash, i);
	    }
	}
	return end();
    }

    // TODO: count the size of the free list so this function is constant
    // time rather than linear in the number of currently free entries.
    uint32_t size() const {
	uint32_t size = chains.size();
	for(int32_t i = free_list; i != -1; i = chains[i].next) {
	    size--;
	}
	return size;
    }

    bool empty() const {
	return size() == 0;
    }

    explicit HashTable(const HashTable &__in) {
	assign(__in);
    }

    HashTable &
    operator=(const HashTable &__in) {
	return assign(__in);
    }
    
    void reserve(unsigned expected_entries) {
	// assertion lets us resize the entry points without thinking about
	// handling existing bits.
	INVARIANT(chains.size() == 0,
		  "have to reserve() before putting anything in hash table");
	chains.reserve(expected_entries);
	unsigned new_entry_size = (unsigned)(expected_entries / target_chain_length);
	unsigned i;
	for(i=0; HashTable_prime_list[i] < new_entry_size; ++i) {
	    SINVARIANT(HashTable_prime_list[i] != 0);
	}
	unsigned new_size = HashTable_prime_list[i];

	entry_points.resize(new_size, -1);
    }

    uint32_t available() {
	return chains.available();
    }

    size_t memoryUsage() const {
	return sizeof(hte) * chains.capacity() + sizeof(int) * entry_points.capacity();
    }

    // this is useful if you want to build up a big table of data, but then
    // want to store it on disk sorted, of course once you sort the vector,
    // you've just destroyed the hash table, but the only other choice
    // is to hash pointers to the data, which loses more memory space.
    // The vector needs to be dense for this to be safe.

    // TODO: add a "densify" function that will go through and remove things
    // from the end of the hash table and put them in free entries until
    // the table is dense.

    bool dense() {
	return free_list == -1;
    }

    hte_vectorT &unsafeGetRawDataVector() {
	INVARIANT(dense(), "If the hash table isn't dense, then there are false values in the vector.");
	return chains;
    }

private:
    uint32_t hashof(const D &data) const {
	return static_cast<uint32_t>(hashfn(data));
    }

    D *internal_add(const D &data, uint32_t hashof_) {
	if (free_list == -1 && 
	    chains.size() >= target_chain_length * entry_points.size()) {
	    resizeHashTable();
	}

	INVARIANT(entry_points.size() > 0, 
		  "did not call init() already? probably a static hash table, which is not safe, see Lintel/src/tests/init-order-test.C");
	uint32_t hash = hashof_ % entry_points.size();
	if (free_list == -1) {
	    hte v(data, entry_points[hash]);
	    DEBUG_SINVARIANT(hash < entry_points.size());
	    entry_points[hash] = static_cast<int>(chains.size());
	    chains.push_back(v);
	    return &(chains.back().data);
	} else {
	    int32_t loc = free_list;
	    
	    free_list = chains[free_list].next;
	    chains[loc].data = data;
	    chains[loc].next = entry_points[hash];
	    entry_points[hash] = loc;
	    return &chains[loc].data;
	}
    }

    template<class V, class C> static inline V *
    static_internal_lookup(C *me, const D &key, uint32_t hashof) {
	if (me->entry_points.size() == 0) {
	    return NULL;
	}
	uint32_t hash = hashof % me->entry_points.size();
	for(int32_t i=me->entry_points[hash]; i != -1; 
	    i = me->chains[i].next) {
	    if (me->equal(key,me->chains[i].data)) {
		return &(me->chains[i]);
	    }
	}
	return NULL;
    }

    hte *internal_lookup(const D &key, uint32_t hashof) {
	return static_internal_lookup<hte>(this, key, hashof);
    }

    const hte *internal_lookup(const D &key, uint32_t hashof) const {
	return static_internal_lookup<const hte>(this, key, hashof);
    }

    void init(double _tcl) {
	INVARIANT(_tcl > 0, 
		  boost::format("invalid target_chain_length %.6f") % _tcl);

	if (entry_points.capacity() == 0) {
	    // only do this if we haven't already done the init in add()
	    free_list = -1;
	    // make the "have we initted" call in add() only have
	    // to test a single possibility
	    entry_points.reserve(1); 
	}
	target_chain_length = _tcl;
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
	SINVARIANT(free_list == -1);
	uint32_t old_size = entry_points.size();
	uint32_t new_size;
	uint32_t i;
	for(i=0; HashTable_prime_list[i] <= old_size;i++) {
	    SINVARIANT(HashTable_prime_list[i] != 0);
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
	    uint32_t hash = hashof(chains[i].data) % new_size;
	    chains[i].next = entry_points[hash];
	    entry_points[hash] = i;
	}
    }
    
    int32_t free_list; // for chains
    hte_vectorT chains;
    hash_tableT entry_points;
    double target_chain_length;
    HashFn hashfn;
    Equal equal;
};

#endif
