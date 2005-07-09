/*
*******************************************************************************
* 
* File:         StringId.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/StringId.H,v 1.3 2003/07/30 00:17:18 anderse Exp $
* Description:  String to small integer mapping
* Author:       Eric Anderson
* Created:      Thu Jan 20 21:35:20 2000
* Modified:     Wed Jul  9 23:11:03 2003 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      Lintel
* 
* (C) Copyright 2000, Hewlett-Packard Laboratories, all rights reserved.
*******************************************************************************
*/
#ifndef __ERGASTULUM_STRINGID_H
#define __ERGASTULUM_STRINGID_H

#include <string>
#include <map>
#include <HashTable.H>
#include <GC_Alloc.H>

class StringId {
public:
    StringId();
    unsigned int getId(const gcstring &str);
    const gcstring &getString(unsigned int id);
    unsigned int maxId() { return nextid; }

    struct HTE {
	const gcstring *str;
	unsigned int id;
	HTE(const gcstring &_str) : str(&_str), id(0) {}
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
#if 0
    std::map<std::string,unsigned int> idmap;
    std::map<unsigned int,std::string> revmap;
#else
    HashTable<HTE, HTEHashStr, HTEEqualStr> idmap;
    HashTable<HTE, HTEHashId, HTEEqualId> revmap;
#endif

    unsigned int nextid;
};

#endif

