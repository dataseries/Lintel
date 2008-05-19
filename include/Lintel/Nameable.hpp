/* -*-C++-*- */
/*
   (c) Copyright 1994-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Simple names for objects
*/

#ifndef LINTEL_NAMEABLE_HPP
#define LINTEL_NAMEABLE_HPP

#include <string.h>
#include <strings.h>
#include <string>

//////////////////////////////////////////////////////////////////////////////
// A Nameable is mixin for anything that should have a name.
//////////////////////////////////////////////////////////////////////////////

class Nameable {
public:
  Nameable() {};
  virtual ~Nameable() {};

    virtual const char  *name() const = 0;	// returns a printable name
    virtual std::string debugString() const = 0;// returns a freeable std::string
    virtual bool   ofInterest() const = 0;	// true if has a debugString
						//   function worth calling
};


//////////////////////////////////////////////////////////////////////////////
// A Named is a kind of Nameable that actually maintains the name.  (There
// may be kinds of Nameables that maintain the name some other way.)
//////////////////////////////////////////////////////////////////////////////

class Named : public virtual Nameable
{
public:
    Named(const char *name_in):
        my_name(strdup(name_in))
    { }
    Named(std::string name_in):
        my_name(strdup(name_in.c_str()))
    { }

    virtual ~Named() 
    { 
        free((void*) my_name); 
    }

    virtual const char *name() const
    { 
        return my_name; 
    }

    // Return the part of the name after the last ".", or the whole
    // thing if there is no ".":
    virtual const char *lastName() const
    {
        const char* return_value = rindex(my_name, '.');
        if (return_value)
    	    return_value++;         // Found it: Jump past the "."
        else
            return_value = my_name; // Didn't find it: Use whole string
	return return_value;
    }

    virtual std::string debugString() const;

    // Override this if you provide debugString():
    virtual bool ofInterest() const 
    { 
        return 0; 
    }

protected:
  const char *my_name;		// a name for the Named object
};

#endif /* _LINTEL_NAMEABLE_H_INCLUDED */