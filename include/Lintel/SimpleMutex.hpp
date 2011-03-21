/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** \file
    \brief header file for simple mutex classes (usable with and without linking with pthread)
*/

#ifndef LINTEL_SIMPLEMUTEX_HPP
#define LINTEL_SIMPLEMUTEX_HPP

#include <pthread.h>

#include <boost/utility.hpp>

/// \brief a simple mutex, creatable with and without linking with libpthread
///
/// Simple interface to a pthread mutex from C++; safe for use in
/// potentially non-threaded programs.  The PThreadMutex class
/// requires linking with libpthread. Should not be used for a file
/// static variable; use the SIMPLE_MUTEX_SINGLETON macro if you need
/// that.
class SimpleMutex : boost::noncopyable {
public:
    SimpleMutex();

    ~SimpleMutex();
    /// Error checked locking routine
    void lock();
    /// Error checked unlocking routine
    void unlock();
private:
    pthread_mutex_t m;
};

/// \brief A simple scoped lock
///
/// Simple scoped lock, again valid for use in non-threaded programs
/// slight overhead in those programs for the write and the function calls.
class SimpleScopedLock : boost::noncopyable {
public:
    SimpleScopedLock(SimpleMutex &m) : m(m) {
        m.lock();
    }

    ~SimpleScopedLock() {
        m.unlock();
    }

    SimpleMutex &m;
};
        
/// \brief A macro that creates a singleton pattern function 
///   
/// Simple macro to create a singleton pattern; this is useful for having a
/// static mutex in a file.  This pattern is safe from problems with order
/// of initialization, whereas just having a static SimpleMutex would not 
/// be safe. 
#define SIMPLE_MUTEX_SINGLETON(fnname) \
  static SimpleMutex &fnname() { \
     static SimpleMutex singleton; \
     return singleton; \
  } 
#endif 
