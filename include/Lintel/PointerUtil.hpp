/* -*-C++-*- */
/*
   (c) Copyright 2007-2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Utilities for dealing with pointers
*/

#ifndef LINTEL_POINTERUTIL_HPP
#define LINTEL_POINTERUTIL_HPP

#include <boost/shared_ptr.hpp>

#include <Lintel/AssertBoost.hpp>

namespace Lintel {

    /// Function for performing a safe downcast of a basic pointer to
    /// a derived class' pointer.  Guarantees that the cast was
    /// successful.  Will not compile unless the Target class is a
    /// subclass or superclass of the Source class.  For up-casts,
    /// static_cast or simple assignment will be faster.
    template <class Target, class Source>
    inline Target *safe_downcast(Source *x)
    {
	Target *ret = dynamic_cast<Target *>(x);
	INVARIANT(ret == x, boost::format("dynamic downcast failed in %s")
		  % __PRETTY_FUNCTION__);  
	return ret;
    }

    /// Function for performing a safe crosscast of a basic pointer to
    /// a related pointer.  Guarantees that the cast was successful.
    template <class Target, class Source>
    inline Target *safe_crosscast(Source *x)
    {
	Target *ret = dynamic_cast<Target *>(x);
	INVARIANT(ret != 0, boost::format("dynamic crosscast failed in %s")
		  % __PRETTY_FUNCTION__);  
	return ret;
    }

    /// Function for performing a safe downcast of a boost shared
    /// pointer to a derived class' pointer.  Guarantees that the cast
    /// was successful.  Will not compile unless the Target class is a
    /// subclass or superclass of the Source class.  For up-casts,
    /// boost::static_pointer_cast or simple assignment will be faster.
    template <class Target, class Source>
    inline boost::shared_ptr<Target> 
    safe_downcast(boost::shared_ptr<Source> x)
    {
	boost::shared_ptr<Target> ret = boost::dynamic_pointer_cast<Target>(x);

	INVARIANT(ret == x, boost::format("dynamic downcast failed in %s")
		  % __PRETTY_FUNCTION__);  
	return ret;
    }

    /// Function for performing a safe crosscast of a boost shared
    /// pointer to a related pointer.  Guarantees that the cast was
    /// successful.
    template <class Target, class Source>
    inline boost::shared_ptr<Target> 
    safe_crosscast(boost::shared_ptr<Source> x)
    {
	boost::shared_ptr<Target> ret = boost::dynamic_pointer_cast<Target>(x);

	INVARIANT(ret != 0, boost::format("dynamic crosscast failed in %s")
		  % __PRETTY_FUNCTION__);  
	return ret;
    }
}

#endif

