/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Interface to ptheads from C++
*/

#ifndef LINTEL_PTHREAD_HPP
#define LINTEL_PTHREAD_HPP
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <boost/utility.hpp>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/Clock.hpp>

#if __linux__
extern "C" {
  int pthread_mutexattr_setkind_np(pthread_mutexattr_t *attr, int kind);
}
#endif

class PThreadMisc { // should be a namespace if we believe everyone supports those
public:
    static int getCurrentCPU(bool unknown_ok = false);
    static int getNCpus(bool unknown_ok = false);
};	   

/** you can call the start function multiple times on this class, this
 * is potentially useful if your threads don't have any per-thread
 * state.  We don't bother to take out locks in these functions
 * because it's expected people will start all their threads from a
 * "master" thread, so the locking is unnecessary.  
 * technically this is copyable, but it's probably not what you wanted. */

class PThread : boost::noncopyable { 
public:
    PThread();
    virtual ~PThread();

    void setDetached(bool detached = true);
    size_t getStackSize();
    void setStackSize(size_t size);
    size_t getGuardSize();
    void setGuardSize(size_t size);

    // override this function to do whatever work you want in your
    // thread.  Return value will be provided for people joining with
    // a thread.

    virtual void *run() = 0;
    virtual pthread_t start();
    void *join();
    int kill(int sig);

private:
    pthread_t last_tid;
    pthread_attr_t attr;
};

/** PThreadNoSignals is a sublcass of PThread that blocks all signals
 * in created threads.  This is usually what you want -- it is often
 * best to let one thread (e.g. the master thread) take all signals.
 *
 * You may enable any signals that you really do want by calling
 * pthread_sigmask(2) in your run() method.
 *
 * As with PThread, it's expected people will start all their threads
 * from a "master" thread.
 */
class PThreadNoSignals : public PThread {
public:
    virtual pthread_t start();
};

struct PThreadMutex : boost::noncopyable {
    pthread_mutex_t m;
#if COMPILE_DEBUG
#define DEFAULT_ERRORCHECK true
#else
#define DEFAULT_ERRORCHECK false
#endif
    PThreadMutex(bool errorcheck = DEFAULT_ERRORCHECK);
    ~PThreadMutex();
    void lock() {
	if (trylock()) {
	    return;
	}
	int ret = pthread_mutex_lock(&m);
	INVARIANT(ret==0,boost::format("pthread_mutex_lock failed: %s") % strerror(ret));
	++ncontention;
    }
    bool trylock() {
        int ret = pthread_mutex_trylock(&m);
        if (ret == 0) {
            return true;
        }
        INVARIANT(ret == EBUSY,
		  boost::format("Invalid error(%d,%s) on trylock") % ret % strerror(ret));
        return false;
    }

    void unlock() {
	int ret = pthread_mutex_unlock(&m);
	INVARIANT(ret==0,
		  boost::format("pthread_mutex_unlock failed: %s")
		  % strerror(ret));
    }
    
#if HPUX_ACC
    // so amazingly non portable, but seems to work
    bool islocked() {
	return ((char *)&m)[67] == 0;
    }
#endif
    int ncontention;

    std::string debugInfo();
};

// Allocate this as an automatic in your stack frame.  It will
// take the mutex on construction and drop it on destruction,
// when the stack frame goes out of scope.
class PThreadAutoLocker : boost::noncopyable {
public:
    explicit PThreadAutoLocker(PThreadMutex &_lock) 
	: lock(_lock)
    {
	lock.lock();
    }

    ~PThreadAutoLocker()
    {
	lock.unlock();
    }

private:
    PThreadMutex &lock;
};

struct PThreadCond : boost::noncopyable {
    pthread_cond_t c;
    PThreadCond() {
	INVARIANT(pthread_cond_init(&c,NULL)==0,"fatal");
    }
    ~PThreadCond() {
	INVARIANT(pthread_cond_destroy(&c)==0,"fatal");
    }
    void signal() {
	INVARIANT(pthread_cond_signal(&c)==0,"fatal");
    }
    void broadcast() {
	INVARIANT(pthread_cond_broadcast(&c)==0,"fatal");
    }
    void wait(PThreadMutex &m) {
	int ret = pthread_cond_wait(&c,&m.m);
	INVARIANT(ret == 0,boost::format("fatal, ret=%d: %s") % ret % strerror(ret));
    }
    bool timedwait(PThreadMutex &m, Clock::Tll us) { // true on timeout
        struct timespec abstime;
        Clock::Tll timeout = Clock::todll() + us;
        abstime.tv_sec = (time_t)(timeout / 1000000);
        abstime.tv_nsec = (time_t)((timeout % 1000000) * 1000);
        int ret = pthread_cond_timedwait(&c,&m.m,&abstime);
        INVARIANT(ret == 0 || ret == ETIMEDOUT,"fatal");
        return ret == ETIMEDOUT ? true : false;
    }
};

#endif
