/* -*-C++-*- */
/*
   (c) Copyright 2004-2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Utilities for dealing with libxml2 bits
*/

#ifndef LINTEL_XMLUTIL_HPP
#define LINTEL_XMLUTIL_HPP

#include <string>
#include <libxml/tree.h>

namespace Lintel {

/// get the value for an attribute of an element.  Returns an empty
/// string if the attribute doesn't exist (so you can't actually tell
/// the difference between an attribute not existing and it set to the
/// empty string)
std::string strGetXMLAttr(xmlNodePtr cur, const std::string &attr_name);

};

#endif
