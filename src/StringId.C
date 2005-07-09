/*
*******************************************************************************
* 
* File:         StringId.C
* RCS:          $Header: /mount/cello/cvs/Lintel/src/StringId.C,v 1.3 2003/07/30 00:17:18 anderse Exp $
* Description:  String to small integer mapping
* Author:       Eric Anderson
* Created:      Sat Mar 11 19:35:04 2000
* Modified:     Wed Jul  9 23:12:17 2003 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      Lintel
* 
* (C) Copyright 2000, Hewlett-Packard Laboratories, all rights reserved.
*******************************************************************************
*/
#include <StringId.H>
#include <LintelAssert.H>

StringId::StringId()
    : revmap(4.0) // equal is very fast, so use longer chains
{
    nextid = 1;
}

unsigned int
StringId::getId(const gcstring &str)
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
	gcstring *alloc = new gcstring(str);
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

const gcstring &
StringId::getString(unsigned int id)
{
#if 0    
    AssertAlways(id < nextid,("Fatal Error"));
    return revmap[id];
#else
    HTE ent(id);
    const HTE *d = revmap.lookup(ent);
    AssertAlways(d != NULL,("internal error id %u has no std::string?!\n",id));
    return *(d->str);
#endif

}

