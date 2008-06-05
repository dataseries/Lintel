/* -*-C++-*- */
/*
   (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <Lintel/XMLUtil.hpp>

using namespace std;

namespace lintel {

string strGetXMLAttr(xmlNodePtr cur, const string &_attr_name)
{
    const xmlChar *attr_name
	= reinterpret_cast<const xmlChar *>(_attr_name.c_str());
    xmlChar *option = xmlGetProp(cur, attr_name);
				 
    if (option == NULL) {
	return "";
    } else {
	string ret(reinterpret_cast<char *>(option));
	xmlFree(option);
	return ret;
    }
}

}
