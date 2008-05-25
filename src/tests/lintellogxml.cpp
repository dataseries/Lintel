/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Testing for LintelLogXML
*/

#include <Lintel/AssertBoost.hpp>
#include <Lintel/XMLUtil.hpp>
#include <Lintel/LintelLogXML.hpp>

using namespace std;

int main()
{
    string xml("<foo category=\"test\" attr2=\"whoo\" />");

    xmlDocPtr doc = xmlReadMemory(xml.c_str(), xml.size(), "foo.xml", NULL, 0);
    
    xmlNodePtr root = xmlDocGetRootElement(doc);
    LintelLogXML::parseXML(root);

    SINVARIANT(LintelLog::wouldDebug("test"));
    SINVARIANT(!LintelLog::wouldDebug("test", 2));

    xml = "<foo category=\"test2\" level=\"3\" />";

    doc = xmlReadMemory(xml.c_str(), xml.size(), "foo.xml", NULL, 0);
    
    root = xmlDocGetRootElement(doc);
    LintelLogXML::parseXML(root);

    SINVARIANT(LintelLog::wouldDebug("test2", 3));
    SINVARIANT(!LintelLog::wouldDebug("test2", 4));

    return 0;
}
