#include <Lintel/Clock.hpp>

#include <Lintel/PThread.hpp>

pthread_key_t Clock::per_thread_clock;
static bool did_per_thread_init;

namespace {
    PThreadScopedOnlyMutex mutex;
}

void Clock::perThreadInit() {
    PThreadScopedLock lock(mutex);
    
    INVARIANT(!did_per_thread_init, "already did Clock::perThreadInit");
    did_per_thread_init = true;
    INVARIANT(pthread_key_create(&per_thread_clock, NULL) == 0, "bad");
    Clock::calibrateClock();
}

Clock &Clock::perThread() {
    INVARIANT(clock_rate > 0 && did_per_thread_init, 
	      "you need to call Clock::perThreadInit before calling Clock::perThread()");
    Clock *c = static_cast<Clock *>(pthread_getspecific(per_thread_clock));
    if (c == NULL) {
	c = new Clock;
	pthread_setspecific(per_thread_clock, c);
    }
    return *c;
}
