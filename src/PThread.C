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
