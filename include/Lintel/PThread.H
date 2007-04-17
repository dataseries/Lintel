/*
   (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Interface to ptheads from C++
*/

#ifndef __PTHREAD_H
#define __PTHREAD_H
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <Lintel/AssertBoost.H>
#include <Lintel/Clock.H>

#if __linux__
extern "C" {
  int pthread_mutexattr_setkind_np(pthread_mutexattr_t *attr, int kind);
}
#endif

class PThreadMisc { // should be a namespace if we believe everyone supports those
public:
    static int getCurrentCPU() {
#if HPUX_ACC
	pthread_spu_t answer;
	INVARIANT(pthread_processor_id_np(PTHREAD_GETCURRENTSPU_NP,
					     &answer,0) == 0,
		  boost::format("Failed: %s") % strerror(errno));
	return (int)answer;
#else
	return -1;
#endif
    }
};	   

/** you can call the start function multiple times on this class, this
 * is potentially useful if your threads don't have any per-thread
 * state.  We don't bother to take out locks in these functions
 * because it's expected people will start all their threads from a
 * "master" thread, so the locking is unnecessary.  */
class PThread {
public:
    PThread();
    virtual ~PThread();

    void setDetached(bool detached = true);

    // override this function to do whatever work you want in your
    // thread.  Return value will be provided for people joining with
    // a thread.

    virtual void *run() = 0;
    virtual pthread_t start();
    void *join();

private:
    pthread_t last_tid;
    pthread_attr_t attr;

    PThread(const PThread &);		   // no copy ctor
    PThread &operator=(const PThread &);   // no assign op
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

struct PThreadMutex {
    pthread_mutex_t m;
#if COMPILE_DEBUG
#define DEFAULT_ERRORCHECK true
#else
#define DEFAULT_ERRORCHECK false
#endif
    PThreadMutex(bool errorcheck = DEFAULT_ERRORCHECK) : ncontention(0) {
	pthread_mutexattr_t attr;
	INVARIANT(pthread_mutexattr_init(&attr)==0,"fatal");
#if __linux__
	if (errorcheck) {
	    INVARIANT(pthread_mutexattr_setkind_np(&attr,
						      PTHREAD_MUTEX_ERRORCHECK_NP)==0,
			 "fatal");
	}
#endif
#if HPUX_ACC
	if (errorcheck) {
	    INVARIANT(pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK)==0,
			 "fatal");
	}
#endif
	INVARIANT(pthread_mutex_init(&m,&attr)==0,"fatal");
    }
    ~PThreadMutex() {
	int ret = pthread_mutex_destroy(&m);
	INVARIANT(ret == 0,
		  boost::format("unable to destroy mutex: %s") % strerror(ret));
    }
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
	INVARIANT(pthread_mutex_unlock(&m)==0,"Fatal Error");
    }
    
#if HPUX_ACC
    // so amazingly non portable, but seems to work
    bool islocked() {
	return ((char *)&m)[67] == 0;
    }
#endif
    int ncontention;

private:
    PThreadMutex(const PThreadMutex &);		     // no copy ctor
    PThreadMutex &operator=(const PThreadMutex &);   // no assign op
};

// Allocate this as an automatic in your stack frame.  It will
// take the mutex on construction and drop it on destruction,
// when the stack frame goes out of scope.
class PThreadAutoLocker {
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

    PThreadAutoLocker(const PThreadAutoLocker &); // nope
    PThreadAutoLocker &operator=(const PThreadAutoLocker &); // nope
};

struct PThreadCond {
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

private:
    PThreadCond(const PThreadCond &);		   // no copy ctor
    PThreadCond &operator=(const PThreadCond &);   // no assign op
};

#endif
