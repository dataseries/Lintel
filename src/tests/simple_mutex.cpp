/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <Lintel/SimpleMutex.hpp>

SIMPLE_MUTEX_SINGLETON(test);

void scopedLock(SimpleMutex &m) {
    SimpleScopedLock lock(m);
}

int main() {
    SimpleMutex test2;

    scopedLock(test());
    scopedLock(test2);

    test().lock();
    test().unlock();
    
    test2.lock();
    test2.unlock();

    return 0;
}

    
