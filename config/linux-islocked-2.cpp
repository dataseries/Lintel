#include <pthread.h>
int main()
{
    pthread_mutex_t m;
    
    return m.__data.__lock != 0;
}
