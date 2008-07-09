/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Testing for PThread
*/

#include <Lintel/PThread.hpp>

void test_isLocked() {
    PThreadMutex mutex;

    mutex.lock();
    SINVARIANT(mutex.trylock() == false);
    mutex.unlock();
    SINVARIANT(mutex.trylock() == true);
    mutex.unlock();

#if LINTEL_PTHREAD_ISLOCKED_AVAILABLE    
    SINVARIANT(mutex.isLocked() == false);
    mutex.lock();
    SINVARIANT(mutex.isLocked() == true);
    mutex.unlock();
    SINVARIANT(mutex.isLocked() == false);
#endif
}

int main(int argc, char *argv[]) {
    test_isLocked();
    return 0;
}
