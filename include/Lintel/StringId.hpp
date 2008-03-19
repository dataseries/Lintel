/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    String to small integer mapping
*/

#ifndef LINTEL_STRINGID_HPP
#define LINTEL_STRINGID_HPP

#include <string>
#include <map>

#include <Lintel/HashTable.hpp>

class StringId {
public:
    StringId();
    unsigned int getId(const std::string &str);
    const std::string &getString(unsigned int id);
    unsigned int maxId() { return nextid; }

    struct HTE {
	const std::string *str;
	unsigned int id;
	HTE(const std::string &_str) : str(&_str), id(0) {}
	HTE(const unsigned int _id) : id(_id) {}
    };
    class HTEHashStr {
    public:
	unsigned int operator()(const HTE &k) {
	    return HashTable_hashbytes(k.str->data(),k.str->size());
	}
    };
    class HTEEqualStr {
    public:
	bool operator()(const HTE &a, const HTE &b) {
	    return *(a.str) == *(b.str);
	}
    };
    class HTEHashId {
    public:
	unsigned int operator()(const HTE &k) {
	    return k.id;
	}
    };
    class HTEEqualId {
    public:
	bool operator()(const HTE &a, const HTE &b) {
	    return a.id == b.id;
	}
    };

private:
    HashTable<HTE, HTEHashStr, HTEEqualStr> idmap;
    HashTable<HTE, HTEHashId, HTEEqualId> revmap;

    unsigned int nextid;
};

#endif

