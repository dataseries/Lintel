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

#include <Lintel/LintelAssert.H>
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
	AssertAlways(pthread_processor_id_np(PTHREAD_GETCURRENTSPU_NP,
					     &answer,0) == 0,
		     ("Failed: %s\n",strerror(errno)));
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
	AssertAlways(pthread_mutexattr_init(&attr)==0,("fatal\n"));
#if __linux__
	if (errorcheck) {
	    AssertAlways(pthread_mutexattr_setkind_np(&attr,
						      PTHREAD_MUTEX_ERRORCHECK_NP)==0,
			 ("fatal\n"));
	}
#endif
#if HPUX_ACC
	if (errorcheck) {
	    AssertAlways(pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK)==0,
			 ("fatal\n"));
	}
#endif
	AssertAlways(pthread_mutex_init(&m,&attr)==0,("fatal\n"));
    }
    ~PThreadMutex() {
	int ret = pthread_mutex_destroy(&m);
	AssertAlways(ret == 0,
		     ("unable to destroy mutex: %s\n",strerror(ret)));
    }
    void lock() {
	if (trylock()) {
	    return;
	}
	int ret = pthread_mutex_lock(&m);
	AssertAlways(ret==0,("pthread_mutex_lock failed: %s",strerror(ret)));
	++ncontention;
    }
    bool trylock() {
        int ret = pthread_mutex_trylock(&m);
        if (ret == 0) {
            return true;
        }
        AssertAlways(ret == EBUSY,
                     ("Invalid error(%d,%s) on trylock",ret,strerror(ret)));
        return false;
    }
    void unlock() {
	AssertAlways(pthread_mutex_unlock(&m)==0,("Fatal Error"));
    }
    
#if HPUX_ACC
    // so amazingly non portable, but seems to work
    bool islocked() {
	return ((char *)&m)[67] == 0;
    }
#endif
    int ncontention;
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
	AssertAlways(pthread_cond_init(&c,NULL)==0,("fatal\n"));
    }
    ~PThreadCond() {
	AssertAlways(pthread_cond_destroy(&c)==0,("fatal\n"));
    }
    void signal() {
	AssertAlways(pthread_cond_signal(&c)==0,("fatal\n"));
    }
    void broadcast() {
	AssertAlways(pthread_cond_broadcast(&c)==0,("fatal\n"));
    }
    void wait(PThreadMutex &m) {
	int ret = pthread_cond_wait(&c,&m.m);
	AssertAlways(ret == 0,("fatal, ret=%d: %s\n",ret,strerror(ret)));
    }
    bool timedwait(PThreadMutex &m, Clock::Tll us) { // true on timeout
        struct timespec abstime;
        Clock::Tll timeout = Clock::todll() + us;
        abstime.tv_sec = (time_t)(timeout / 1000000);
        abstime.tv_nsec = (time_t)((timeout % 1000000) * 1000);
        int ret = pthread_cond_timedwait(&c,&m.m,&abstime);
        AssertAlways(ret == 0 || ret == ETIMEDOUT,("fatal\n"));
        return ret == ETIMEDOUT ? true : false;
    }
};

#endif


    
