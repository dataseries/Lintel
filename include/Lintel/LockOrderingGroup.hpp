/* -*- C++ -*-
   (c) Copyright 2000-2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief header file for LockOrderingGroup class
*/

#ifndef LINTEL_LOCK_ORDERING_GROUP_HPP
#define LINTEL_LOCK_ORDERING_GROUP_HPP

#include <boost/utility.hpp>
#include <boost/thread/tss.hpp>

/// \brief Class for defining and tracking the ordering across a group of locks
///
/// Use this class to define a set of related mutexes/locks that will
/// be tested to verify that the locking is all globally ordered.
/// Right now you can only use this with the ScopedLock/Unlock classes
/// as they will properly manipulate the lock ordering when
/// taking/releasing locks.  Lock ordering levels are defined as
/// higher is more important, so it is ok to override a lower level
/// lock with a higher level one, but not in the reverse.
class LockOrderingGroup : boost::noncopyable {
public:
    LockOrderingGroup() { 
	prepare();
    }

    /// prepare a thread to use this lock ordering group; safe to call
    /// multiple times -- it will do nothing if a group is already
    /// prepared.
    void prepare() {
	if (cur_level.get() == NULL) {
	    cur_level.reset(new double);
	    *cur_level = 0;
	}
    }

    double curLevel() const {
        SINVARIANT(cur_level.get() != NULL);
	return *cur_level;
    }

    boost::thread_specific_ptr<double> cur_level;

    /// go down to a further depth of lock scoping; public so that it can
    /// be used by other code, but you should be careful about calling
    /// these.  You really should just use PThreadScopedLock.
    double scopeDown(double new_level) {
	double old_level = *cur_level;
	INVARIANT(old_level < new_level, 
		  boost::format("lock scope down failed: %.6g >= %.6g") 
		  % old_level % new_level);
	*cur_level = new_level;
	return old_level;
    }

    /// return back to a previous depth of lock scoping; public so that it
    /// can be used by other code, but you should be careful about
    /// calling these. You really should just use PThreadScopedLock.
    void scopeUp(double down_level, double old_level) {
	INVARIANT(*cur_level == down_level,
		  boost::format("lock scope up failed: %.6g != %.6g") 
		  % *cur_level % down_level);
	*cur_level = old_level;
    }
};
	
#endif
