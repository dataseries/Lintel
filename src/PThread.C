/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    PThread implementation
*/

#include <PThread.H>
#include <signal.h>

// Platform discrimination
//
// GLIBC_PTHREADS is presumed sufficient to always identify glibc
//
// GLIB_OLD_PTHREADS is not known to be sufficent to detect where the
// old code below works and doesn't, but it is good enough for the few
// platforms tested.

#define GLIBC_PTHREADS (defined(_BITS_PTHREADTYPES_H))
#define GLIBC_OLD_PTHREADS  (!defined(__SIZEOF_PTHREAD_MUTEX_T))


PThread::PThread()
{
    int ret = pthread_attr_init(&attr);
    INVARIANT(ret == 0,boost::format("pthread_attr_init failed: %s") % strerror(ret));
    memset(&last_tid,sizeof(last_tid),0);
}

PThread::~PThread()
{
}

void
PThread::setDetached(bool detached)
{
    int ret = pthread_attr_setdetachstate(&attr, detached ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE);
    INVARIANT(ret == 0,boost::format("pthread_attr_setdetachstate failed: %s") % strerror(ret));
}

static void *
pthread_starter(void *arg)
{
    PThread *obj = (PThread *)arg;
    return obj->run();
}

pthread_t
PThread::start()
{
    pthread_t tid;
    int ret = pthread_create(&tid,&attr,pthread_starter,this);

    INVARIANT(ret == 0,boost::format("pthread_create failed: %s") % strerror(ret));
    
    last_tid = tid;
    return tid;
}

void *
PThread::join()
{
    void *retval = NULL;
    int tmp = pthread_join(last_tid, &retval);
    INVARIANT(tmp == 0,boost::format("pthread_join failed: %s") % strerror(tmp));
    return retval;
}

int
PThread::kill(int sig)
{
    int rc = pthread_kill(last_tid, sig);
    return rc;
}

#if GLIBC_PTHREADS && GLIBC_OLD_PTHREADS
#define PTHREADMUTEX_PLATFORM_DEBUG_INFO_DEFINED 1
std::string
PThreadMutex::debugInfo()
{
    // this is for some version of glibc prior to 2.5 (but maybe earlier)
    // code by ea
    return (boost::format("mutex %p: recursive-depth %d owner %p kind %d status %d spinlock %d") 
	    % reinterpret_cast<void *>(&m) % m.__m_count 
	    % reinterpret_cast<void *>(m.__m_owner) % m.__m_kind 
	    % m.__m_lock.__status 
	    % static_cast<uint32_t>(m.__m_lock.__spinlock)).str();
}
#endif

#if GLIBC_PTHREADS && !GLIBC_OLD_PTHREADS
#define PTHREADMUTEX_PLATFORM_DEBUG_INFO_DEFINED 1
std::string
PThreadMutex::debugInfo()
{
    // glibc 2.5 (but maybe earlier)
    // code by ch
    return (boost::format("mutex %p: recursive-depth %d owner %p kind %d lock %d") 
	    % reinterpret_cast<void *>(&m) 
	    % m.__data.__count 
	    % reinterpret_cast<void *>(m.__data.__owner) 
	    % m.__data.__kind 
	    % m.__data.__lock).str();
}
#endif

// fallback definition
#if !defined(PTHREADMUTEX_PLATFORM_DEBUG_INFO_DEFINED)
std::string
PThreadMutex::debugInfo()
{
    return "no debugging information available on this platform";
}
#endif


pthread_t
PThreadNoSignals::start()
{
    sigset_t oldset, newset;
    sigfillset(&newset);

    // don't catch any signals in this thread
    // n.b. we're expecting this to be called from a master thread, 
    // so no contention for the sigmask should occur
    pthread_sigmask(SIG_BLOCK, &newset, &oldset);
    pthread_t tid = PThread::start();
    pthread_sigmask(SIG_SETMASK, &oldset, 0);
    return tid;
}
