#include <pthread.h>
#include <string.h>

// islocked === m.__data.__lock != 0
int main() {
    pthread_mutexattr_t attr;
    pthread_mutex_t m;
    
    memset(&attr, 0, sizeof(attr)); // needed on cygwin
    pthread_mutexattr_init(&attr); // make sure we're linking with -lpthread

    pthread_mutex_init(&m, &attr);
    
    if (m.__data.__lock != 0)
	return 1; // not initialized to 0

    pthread_mutex_lock(&m);

    if (m.__data.__lock == 0) 
	return 1; // not changed from 0 after lock

    return 0; 
}
