/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief More portable definitions of standard integer types (windows only)
*/

/*
    Essentially, we just use boost, and ensure that the types are 'use'ed
    to appear in the namespace without the boost qualifications
*/

#ifndef LINTEL_WINDOWS_STDINT_HPP
#define LINTEL_WINDOWS_STDINT_HPP

#include <boost/cstdint.hpp>

using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;

#endif // LINTEL_WINDOWS_STDINT_HPP
