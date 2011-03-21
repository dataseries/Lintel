/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

// TODO: move this into unstable.

/** @file
    \brief An initial attempt at working out invariant handling through exceptions.  
    
    A version of assertions that throws an exception so the assertions
    can be caught.  This approach was incomplete, and may be deprecated.
*/

#ifndef LINTEL_ASSERTEXCEPTION_HPP
#define LINTEL_ASSERTEXCEPTION_HPP

#include <string>
#include <exception>

/** \brief A class and macros for capturing assertions as exceptions.

    A version of assertions that throws an exception so the assertions
    can be caught.  This approach was incomplete, and may be deprecated.
*/
    
class AssertExceptionT : public std::exception {
public:
    AssertExceptionT(const std::string &_condition, 
		     const std::string &_message,
		     const char *_filename, const int _lineno)
	: condition(_condition), message(_message), filename(_filename), lineno(_lineno) { 
    }
    virtual ~AssertExceptionT() throw();

    static std::string stringPrintF(const char *format, ...) 
#ifdef __GNUC__
       __attribute__ ((format (printf,1,2)))
#endif
	;
    const std::string condition;
    const std::string message;
    const char *filename;
    const int lineno;
};

#define SubtypeAssertException(subtype, condition) \
  ( (condition) ? (void)0 : throw subtype (#condition, "", __FILE__, __LINE__) )
#define SubtypeAssertExceptionMsg(subtype, condition, message) \
  ( (condition) ? (void)0 : throw subtype (#condition, (AssertExceptionT::stringPrintF message), __FILE__, __LINE__) )

#define AssertException(condition) SubtypeAssertException(AssertExceptionT, condition)
#define AssertExceptionMsg(condition, message) SubtypeAssertExceptionMsg(AssertExceptionT, condition, message)

  
#endif

