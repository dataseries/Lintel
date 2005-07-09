/*
*******************************************************************************
* 
* File:         PThread.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/PThread.H,v 1.11 2004/02/26 05:03:59 anderse Exp $
* Description:  Interface to ptheads from C++
* Author:       Eric Anderson
* Created:      Wed Mar  8 15:19:54 2000
* Modified:     Wed Nov 12 20:11:53 2003 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      Lintel
* 
* (C) Copyright 2000, Hewlett-Packard Laboratories, all rights reserved.
*******************************************************************************
*/
#ifndef __PTHREAD_H
#define __PTHREAD_H
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <LintelAssert.H>
#include <Clock.H>

#if __linux__
extern "C" {
  int pthread_mutexattr_setkind_np(pthread_mutexattr_t *attr, int kind);
}
#endif

class PThread {
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
		     ("fatal: %s\n",strerror(ret)));
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
	AssertAlways(pthread_cond_wait(&c,&m.m)==0,("fatal\n"));
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


    
