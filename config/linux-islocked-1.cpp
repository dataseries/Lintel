#include <pthread.h>
int main()
{
    pthread_mutex_t m;
    
    return m.__m_lock.__status != 0;
}
