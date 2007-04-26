/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    PThread implementation
*/

#include <PThread.H>
#include <signal.h>

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

std::string
PThreadMutex::debugInfo()
{
#if 0 && defined(_BITS_PTHREADTYPES_H) 
    // glibc...

not quite glibc...

../../../../Lintel/src/PThread.C: In member function 'std::string PThreadMutex::debugInfo()':
../../../../Lintel/src/PThread.C:66: error: 'union pthread_mutex_t' has no member named '__m_count'
../../../../Lintel/src/PThread.C:67: error: 'union pthread_mutex_t' has no member named '__m_owner'
../../../../Lintel/src/PThread.C:67: error: 'union pthread_mutex_t' has no member named '__m_kind'
../../../../Lintel/src/PThread.C:68: error: 'union pthread_mutex_t' has no member named '__m_lock'
../../../../Lintel/src/PThread.C:69: error: 'union pthread_mutex_t' has no member named '__m_lock'

ch@gato:~/src/ticoli$ dpkg --search /usr/include/bits/pthreadtypes.h
libc6-dev: /usr/include/bits/pthreadtypes.h
ch@gato:~/src/ticoli$ dpkg --status libc6-dev
Package: libc6-dev
Status: install ok installed
Priority: standard
Section: libdevel
Installed-Size: 12236
Maintainer: Ubuntu Core developers <ubuntu-devel-discuss@lists.ubuntu.com>
Architecture: i386
Source: glibc
Version: 2.5-0ubuntu14
Replaces: man-db (<= 2.3.10-41), gettext (<= 0.10.26-1), ppp (<= 2.2.0f-24), libgdbmg1-dev (<= 1.7.3-24), ldso (<= 1.9.11-9), netkit-rpc, netbase (<< 4.0), kerberos4kth-dev (<< 1.2.2-10), libc6-prof (<< 2.3.5-2)
Provides: libc-dev
Depends: libc6 (= 2.5-0ubuntu14), linux-libc-dev
Recommends: gcc | c-compiler
Suggests: glibc-doc, manpages-dev
Conflicts: libstdc++2.10-dev (<< 1:2.95.2-15), gcc-2.95 (<< 1:2.95.3-9), libpthread0-dev, libdl1-dev, libdb1-dev, libgdbm1-dev, libc6-dev (<< 2.0.110-1), locales (<< 2.1.3-5), libstdc++2.9-dev, netkit-rpc, libc-dev
Description: GNU C Library: Development Libraries and Header Files
 Contains the symlinks, headers, and object files needed to compile
 and link programs which use the standard C library.
Original-Maintainer: GNU Libc Maintainers <debian-glibc@lists.debian.org>
ch@gato:~/src/ticoli$ 

    return (boost::format("mutex %p: recursive-depth %d owner %p kind %d status %d spinlock %d") 
	    % reinterpret_cast<void *>(&m) % m.__m_count 
	    % reinterpret_cast<void *>(m.__m_owner) % m.__m_kind 
	    % m.__m_lock.__status 
	    % static_cast<uint32_t>(m.__m_lock.__spinlock)).str();
#else
    return "no debugging information available";
#endif
}

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
