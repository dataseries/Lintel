/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    AssertException implementation
*/

#include <stdarg.h>

#include <Lintel/AssertException.hpp>

AssertExceptionT::~AssertExceptionT() throw()
{
}

std::string
AssertExceptionT::stringPrintF(const char *format, ...) 
{
  // could dynamically size up to get a long enough string
  std::string ret;

  ret.resize(1024);
  va_list ap;
  va_start(ap, format);

  int actual_size = vsnprintf(&ret[0], ret.size(), format, ap);

  va_end(ap);
  ret.resize(actual_size);
  return ret;
}

