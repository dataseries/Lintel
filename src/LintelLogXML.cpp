/* -*-C++-*- */
/*
   (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <Lintel/LintelLogXML.hpp>
#include <Lintel/StringUtil.hpp>
#include <Lintel/XMLUtil.hpp>

using namespace std;
using namespace Lintel;

void LintelLogXML::parseXML(xmlNodePtr n) 
{
    string category = strGetXMLAttr(n, "category");
    string level_str = strGetXMLAttr(n, "level");
    
    uint8_t level = 1;
    if (!level_str.empty()) {
	uint32_t tmp = stringToUInt32(level_str);
	SINVARIANT(tmp < 255);
	level = static_cast<uint8_t>(tmp);
    }

    LintelLog::setDebugLevel(category, level);
}

