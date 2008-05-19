/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    String to small integer mapping
*/

#include <Lintel/AssertBoost.hpp>
#include <Lintel/StringId.hpp>

StringId::StringId()
    : revmap(4.0) // equal is very fast, so use longer chains
{
    nextid = 1;
}

unsigned int
StringId::getId(const std::string &str)
{
#if 0
    unsigned int tmp = idmap[str];
    if (tmp > 0)
	return tmp;
    idmap[str] = nextid;
    revmap[nextid] = str;
    tmp = nextid;
    nextid += 1;
    return tmp;
#else
    HTE ent(str);
    const HTE *d = idmap.lookup(ent);
    if (d == NULL) {
	std::string *alloc = new std::string(str);
	ent.str = alloc;
	ent.id = nextid;
	idmap.add(ent);
	revmap.add(ent);
	nextid += 1;
    } else {
	ent.id = d->id;
    }
    return ent.id;
#endif
}

const std::string &
StringId::getString(unsigned int id)
{
#if 0    
    SINVARIANT(id < nextid);
    return revmap[id];
#else
    HTE ent(id);
    const HTE *d = revmap.lookup(ent);
    INVARIANT(d != NULL,
	      boost::format("internal error id %u has no std::string?!") % id);
    return *(d->str);
#endif

}

