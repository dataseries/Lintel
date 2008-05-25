/* -*-C++-*- */
/*
   (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    XML parsing for logging
*/

#ifndef LINTEL_LOG_XML_HPP
#define LINTEL_LOG_XML_HPP

#include <Lintel/LintelLog.hpp>
#include <Lintel/XMLUtil.hpp>

class LintelLogXML : public LintelLog {
public:
    /// parse an XML element describing a debug logging level.
    /// Expects the node to have an attribute with the name
    /// "category", and an optional attribute with the name "level".
    static void parseXML(xmlNodePtr node);
};

#endif
