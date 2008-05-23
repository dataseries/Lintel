/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    implementation
*/

#include <Lintel/AssertBoost.hpp>
#include <Lintel/SimpleMutex.hpp>

SimpleMutex::SimpleMutex()
{ 
    pthread_mutex_init(&m, NULL);
}

SimpleMutex::~SimpleMutex()
{ 
    int ret = pthread_mutex_destroy(&m); 
    INVARIANT(ret == 0, boost::format("pthread_mutex_destroy failed: %s")
	      % strerror(ret));
}

void SimpleMutex::lock() 
{
    int ret = pthread_mutex_lock(&m);
    INVARIANT(ret == 0, boost::format("pthread_mutex_lock failed: %s")
	      % strerror(ret));
}

void SimpleMutex::unlock() 
{
    int ret = pthread_mutex_unlock(&m);
    INVARIANT(ret == 0, boost::format("pthread_mutex_unlock failed: %s")
	      % strerror(ret));
}
