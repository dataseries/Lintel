#include <Lintel/Clock.hpp>

#include <Lintel/PThread.hpp>

pthread_key_t Clock::per_thread_clock;
static bool did_per_thread_init;

namespace {
    PThreadScopedOnlyMutex mutex;
}

void Clock::perThreadInit() {
    PThreadScopedLock lock(mutex);
    // TODO-review: Again, if there are two users of the todcc
    // functionality; it is unreasonable for them to have to
    // coordinate when lintel is in the perfect position to do so for
    // them.
    if (did_per_thread_init) {
	return;
    }

    //INVARIANT(!did_per_thread_init, "already did Clock::perThreadInit");

    did_per_thread_init = true;
    INVARIANT(pthread_key_create(&per_thread_clock, NULL) == 0, "bad");
    Clock::calibrateClock();
}

Clock &Clock::perThread() {
    Clock *c = static_cast<Clock *>(pthread_getspecific(per_thread_clock));
    if (c == NULL) {
	INVARIANT(did_per_thread_init && isCalibrated(),
		  "you need to call Clock::perThreadInit before calling Clock::perThread()");
	c = new Clock;
	pthread_setspecific(per_thread_clock, c);
    }
    return *c;
}
