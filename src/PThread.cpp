/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    PThread implementation
*/

#include <signal.h>

#include <iostream>

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE_CC__)
#define SYSCTL_NCPUS 1
#endif

#ifdef SYSCTL_NCPUS
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include <Lintel/PThread.hpp>
#include <Lintel/SimpleMutex.hpp>
#include <Lintel/StringUtil.hpp>

using namespace std;
using boost::format;

// Platform discrimination
//
// GLIBC_PTHREADS is presumed sufficient to always identify glibc
//
// GLIB_OLD_PTHREADS is not known to be sufficent to detect where the
// old code below works and doesn't, but it is good enough for the few
// platforms tested.

#if (defined(_BITS_PTHREADTYPES_H))
#define GLIBC_PTHREADS 1
#else
#define GLIBC_PTHREADS 0
#endif

#if (!defined(__SIZEOF_PTHREAD_MUTEX_T))
#define GLIBC_OLD_PTHREADS 1
#else
#define GLIBC_OLD_PTHREADS 0
#endif

int PThreadMisc::getCurrentCPU(bool unknown_ok) {
#if HPUX_ACC
	pthread_spu_t answer;
	int err = pthread_processor_id_np(PTHREAD_GETCURRENTSPU_NP,
					  &answer,0);
	INVARIANT(err == 0, format("Failed: %s") % strerror(err));
	return static_cast<int>answer;
#else
	INVARIANT(unknown_ok, "Don't know how to get current cpu");
	return -1;
#endif
}

int PThreadMisc::getNCpus(bool unknown_ok) {
    static SimpleMutex m;
    static unsigned cache_ncpus;

    SimpleScopedLock lock(m);
    // http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
    if (cache_ncpus == 0) {
	// automatically set on windows, and hence cygwin.
        if (const char* result = getenv("NUMBER_OF_PROCESSORS")) {
            cache_ncpus = stringToInteger<uint32_t>(result);
	    INVARIANT(cache_ncpus > 0, "NUMBER_OF_PROCESSORS must be > 0");
#ifdef __linux__
	} else if (1) {
	    ifstream infile("/proc/cpuinfo");

	    INVARIANT(infile.good(), "unable to open /proc/cpuinfo");

	    string line;
	    string processor("processor\t: ");
	    while(!infile.eof()) {
		getline(infile, line);
		if (prefixequal(line,processor)) {
		    ++cache_ncpus;
		}
	    }
	    INVARIANT(cache_ncpus > 0, "no processors found in /proc/cpuinfo");
#endif
#if defined(SYSCTL_NCPUS)
        } else if (1) {
            int name[4];
            int ncpus = 0;
            size_t length = sizeof(ncpus);

            name[0] = CTL_HW;
#if defined HW_AVAILCPU
            name[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

            /* get the number of CPUs from the system */
            CHECKED(sysctl(name, 2, &ncpus, &length, NULL, 0) == 0, "sysctl failed");
            SINVARIANT(length == sizeof(ncpus));
#endif
            if (ncpus < 1) {
                name[1] = HW_NCPU;
                CHECKED(sysctl(name, 2, &ncpus, &length, NULL, 0) == 0, "sysctl failed");
                SINVARIANT(length == sizeof(ncpus));
            }

            if (ncpus < 1) {
                FATAL_ERROR("Can not get number of CPUS on FreeBSD via sysctl");
            }

            cache_ncpus = ncpus;
#endif
	} else {
	    INVARIANT(unknown_ok, "don't know how to get the number of CPUs;"
		      " fix the code or set the NUMBER_OF_PROCESSORS"
		      " environment variable");
	    return -1;
	}
    }
    INVARIANT(cache_ncpus > 0, "huh");
    return cache_ncpus;
}

PThread::PThread() : thread_live(false) {
    // memset needed on cygwin so it doesn't think attr is already initialized
    memset(&attr,0,sizeof(attr));
    int ret = pthread_attr_init(&attr);
    INVARIANT(ret == 0,
	      format("pthread_attr_init failed: %s") % strerror(ret));
    memset(&last_tid,0,sizeof(last_tid));
}

PThread::~PThread() {
    pthread_attr_destroy(&attr);
}

void PThread::setDetached(bool detached) {
    int state = detached ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE;
    int ret = pthread_attr_setdetachstate(&attr, state);
    INVARIANT(ret == 0,format("pthread_attr_setdetachstate failed: %s")
	      % strerror(ret));
}

size_t PThread::getStackSize() {
    size_t size;
    int ret = pthread_attr_getstacksize(&attr, &size);
    INVARIANT(ret == 0, format("pthread_attr_getstacksize failed: %s")
	      % strerror(ret));
    return size;
}

void PThread::setStackSize(size_t size) {
    int ret = pthread_attr_setstacksize(&attr, size);
    INVARIANT(ret == 0, format("pthread_attr_setstacksize failed: %s")
	      % strerror(ret));
}

#ifndef __CYGWIN__
size_t PThread::getGuardSize() {
#ifdef __CYGWIN__
    return 0;
#else
    size_t size;
    int ret = pthread_attr_getguardsize(&attr, &size);
    INVARIANT(ret == 0, format("pthread_attr_getguardsize failed: %s")
	      % strerror(ret));
    return size;
#endif
}

void PThread::setGuardSize(size_t size) {
#ifdef __CYGWIN__
    // nothing to do
#else
    int ret = pthread_attr_setguardsize(&attr, size);
    INVARIANT(ret == 0, format("pthread_attr_setguardsize failed: %s")
	      % strerror(ret));
#endif
}
#endif

static void * pthread_starter(void *arg) {
    PThread *obj = (PThread *)arg;
    return obj->run();
}

pthread_t PThread::start() {
    SINVARIANT(!thread_live);
    pthread_t tid;
    int ret = pthread_create(&tid,&attr,pthread_starter,this);

    INVARIANT(ret == 0, format("pthread_create failed: %s") % strerror(ret));

    thread_live = true;
    last_tid = tid;
    return tid;
}

void * PThread::join() {
    SINVARIANT(thread_live);
    void *retval = NULL;
    int tmp = pthread_join(last_tid, &retval);
    INVARIANT(tmp == 0, format("pthread_join failed: %s") % strerror(tmp));
    thread_live = false;
    return retval;
}

int PThread::kill(int sig) {
    int rc = pthread_kill(last_tid, sig);
    INVARIANT(rc != EINVAL, format("pthread_kill failed: %s") % strerror(rc));
    return rc;
}

PThreadScopedOnlyMutex::PThreadScopedOnlyMutex(bool errorcheck)
    : ncontention(0)
{
    pthread_mutexattr_t attr;
    SINVARIANT(pthread_mutexattr_init(&attr)==0);
#if __linux__
    if (errorcheck) {
	SINVARIANT(pthread_mutexattr_setkind_np(&attr,PTHREAD_MUTEX_ERRORCHECK_NP)==0);
    }
#endif
#if HPUX_ACC
    if (errorcheck) {
	SINVARIANT(pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK)==0);
    }
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE_CC__)
    if (errorcheck) {
        SINVARIANT(pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK)==0);
    }
#endif
    SINVARIANT(pthread_mutex_init(&m,&attr)==0);
}

PThreadScopedOnlyMutex::~PThreadScopedOnlyMutex() {
    int ret = pthread_mutex_destroy(&m);
    INVARIANT(ret == 0,
	      format("unable to destroy mutex: %s") % strerror(ret));
#if LINTEL_DEBUG
    memset(&m, 0x55, sizeof(m));
#endif
}

#if GLIBC_PTHREADS && GLIBC_OLD_PTHREADS
#define PTHREADMUTEX_PLATFORM_DEBUG_INFO_DEFINED 1
string PThreadScopedOnlyMutex::debugInfo() const {
    // this is for some version of glibc prior to 2.5 (but maybe earlier)
    // code by ea
    return (format("mutex %p: recursive-depth %d owner %p kind %d status %d spinlock %d")
	    % reinterpret_cast<const void *>(&m) % m.__m_count
	    % reinterpret_cast<const void *>(m.__m_owner) % m.__m_kind
	    % m.__m_lock.__status
	    % static_cast<uint32_t>(m.__m_lock.__spinlock)).str();
}
#endif

#if GLIBC_PTHREADS && !GLIBC_OLD_PTHREADS
#define PTHREADMUTEX_PLATFORM_DEBUG_INFO_DEFINED 1
string PThreadScopedOnlyMutex::debugInfo() const {
    // glibc 2.5 (but maybe earlier)
    // code by ch
    return (format("mutex %p: recursive-depth %d owner %p kind %d lock %d")
	    % reinterpret_cast<const void *>(&m)
	    % m.__data.__count
	    % reinterpret_cast<const void *>(m.__data.__owner)
	    % m.__data.__kind
	    % m.__data.__lock).str();
}
#endif

// fallback definition
#if !defined(PTHREADMUTEX_PLATFORM_DEBUG_INFO_DEFINED)
string PThreadScopedOnlyMutex::debugInfo() const {
    return "no debugging information available on this platform";
}
#endif

pthread_t PThreadNoSignals::start() {
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

void *PThreadFunction::run() {
    return fn();
}


#if defined(__OpenBSD__) || defined(__APPLE_CC__)
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abs_timeout) {
    int ret;
    struct timespec now, to_sleep;

    to_sleep.tv_sec = 0;
    while ((ret = pthread_mutex_trylock(mutex)) == EBUSY) {
        CHECKED(clock_gettime(CLOCK_REALTIME, &now) == 0, "clock_gettime failed");
        if (now.tv_sec > abs_timeout->tv_sec ||
            (now.tv_sec == abs_timeout->tv_sec && now.tv_nsec >= abs_timeout->tv_nsec)) {
            return ETIMEDOUT;
        }
        to_sleep.tv_nsec = abs_timeout->tv_nsec - now.tv_nsec;
        if (to_sleep.tv_nsec < 0) {
            to_sleep.tv_nsec += 1000 * 1000 * 1000;
        }
        // sleep at most 10ms.
        if (to_sleep.tv_nsec > 10 * 1000 * 1000) {
            to_sleep.tv_nsec = 10 * 1000 * 1000;
        }
        nanosleep(&to_sleep, NULL);
    }
    return ret;
}
#endif

#if defined(__APPLE_CC__)
int clock_gettime(int how, struct timespec *into) {
    struct timeval tp;
    (void)how;
    int ret = gettimeofday(&tp, NULL);
    into->tv_sec = tp.tv_sec;
    into->tv_nsec = tp.tv_usec * 1000;
    return ret;
}
#endif



