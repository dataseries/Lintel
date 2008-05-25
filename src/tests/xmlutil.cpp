/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Testing for XMLUtil
*/

#include <Lintel/AssertBoost.hpp>
#include <Lintel/XMLUtil.hpp>
#include <Lintel/LintelLogXML.hpp>

using namespace std;
using namespace Lintel;

int main()
{
    string xml("<foo attr1=\"\" attr2=\"whoo\" />");

    xmlDocPtr doc = xmlReadMemory(xml.c_str(), xml.size(), "foo.xml", NULL, 0);
    
    xmlNodePtr root = xmlDocGetRootElement(doc);
    
    SINVARIANT(strGetXMLAttr(root, "missing") == "");
    SINVARIANT(strGetXMLAttr(root, "attr1") == "");
    SINVARIANT(strGetXMLAttr(root, "attr2") == "whoo");

    return 0;
}
