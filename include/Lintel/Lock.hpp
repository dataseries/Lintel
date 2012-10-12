#ifndef LINTEL_LOCKS_HPP
#define LINTEL_LOCKS_HPP 

#include <thread>
#include <pthread.h>

#include <AtomicCounter.hpp>

namespace lintel {

class RWPosixLock {
private:
  pthread_rwlock_t rwl;
  RWPosixLock(const RWPosixLock&); //=delete
  void operator=(const RWPosixLock&); //=delete
  //RWPosixLock& operator=(const RWPosixLock&&); // maybe implement
public:
  RWPosixLock():rwl(PTHREAD_RWLOCK_INITIALIZER){}
  void rdlock() { pthread_rwlock_rdlock(&rwl); }
  void wrlock() { pthread_rwlock_wrlock(&rwl); }
  void unlock() { pthread_rwlock_unlock(&rwl); }
};

class PosixLock {
private:
  pthread_mutex_t rwl;
  PosixLock(const PosixLock&); //=delete
  void operator=(const PosixLock&); //=delete
  //PosixLock& operator=(const PosixLock&&); // maybe implement
public:
  PosixLock():rwl(PTHREAD_MUTEX_INITIALIZER){}
  void rdlock() { pthread_mutex_lock(&rwl); }
  void wrlock() { pthread_mutex_lock(&rwl); }
  void unlock() { pthread_mutex_unlock(&rwl); }
};

class PosixSpinLock {
private:
  pthread_spinlock_t rwl;
  PosixSpinLock(const PosixSpinLock&); //=delete
  void operator=(const PosixSpinLock&); //=delete
  //PosixSpinLock& operator=(const PosixSpinLock&&); // maybe implement
public:
  PosixSpinLock(){ pthread_spin_init(&rwl, PTHREAD_PROCESS_SHARED); }
  void rdlock() { pthread_spin_lock(&rwl); }
  void wrlock() { pthread_spin_lock(&rwl); }
  void unlock() { pthread_spin_unlock(&rwl); }
};

class AtomicSpinLock {
private:
  lintel::Atomic<int> rwl;
  AtomicSpinLock(const AtomicSpinLock&); //=delete
  void operator=(const AtomicSpinLock&); //=delete
  //AtomicSpinLock& operator=(const AtomicSpinLock&&); // maybe implement
private:
  void punlock () { rwl = 1; } 

  void lock ()
  {
    int count = 10;
    while (--rwl) {
      if (!--count) { 
	//std::this_thread::yield();
	asm("pause":::);
	count = 10;
      }
    }
    
    /*
    int one = 1;
    while ( !rwl.compare_exchange_strong(one, 0) ) {
      one = 1;
    }*/
  }
   
public:
  AtomicSpinLock(){ punlock(); }
  void rdlock() { lock(); }
  void wrlock() { lock(); }
  void unlock() { punlock(); }
};

class SpinLockYield {
private:
  int rwl __attribute__((__aligned__(64)));
  char padding[64-sizeof(rwl)];
  SpinLockYield(const SpinLockYield&); //=delete
  void operator=(const SpinLockYield&); //=delete
  //SpinLockYield& operator=(const SpinLockYield&&); // maybe implement
private:
  void punlock () 
  { 
    asm("movl $1, %0\n\t": "=m"(rwl)::"memory");
  }

  void lock ()
  {
    for (;;) {
      unsigned int spincount=4;
      asm(
	  "cmpl $1, %0; jne 2f \n\t" //optimize for high contention 4cpu=2.3x. 
	  ".align 16           \n\t" // 1.67x with 4 cpus
	  "1: lock decl %0     \n\t" //or "lock sub $1, %0\n\t"
	  "jz 3f              \n\t"
	  //".subsection 1       \n\t" //with 4 cpu 1.05x
	  ".align 16           \n\t" //sometimes makes 1.1-1.2x, sometimes isn't
	  "2:                  \n\t"
	  "pause               \n\t" // 4 cpus = 2x , 2 cpus = 1.1x
	  "cmpl $1, %0         \n\t" //global test-and-... spin
	  "je 1b               \n\t"
	  //"jmp 1b \n\t"
	  "dec %1              \n\t" 
	  "jnz 2b              \n\t"
	  //"call pthread_yield  \n\t"
	  //"jmp 1b \n\t" 
	  //".previous           \n\t"
	  "3:                  \n\t"
	  : "+m"(rwl), "+r"(spincount)
	  : 
	  : "memory");
      
      if(spincount) {
	break;
      } else {
	pthread_yield(); //std::this_thread::yield();
	spincount = 4;
	continue;
      }
    }
  }
   
public:
  SpinLockYield(){ punlock(); }
  void rdlock() { lock(); }
  void wrlock() { lock(); }
  void unlock() { punlock(); }
};

class NullLock {
private:
  lintel::Atomic<int> rwl;
  NullLock(const NullLock&); //=delete
  void operator=(const NullLock&); //=delete
  //NullLock& operator=(const NullLock&&); // maybe implement
public:
  NullLock() { }
  void rdlock() { }
  void wrlock() { }
  void unlock() { }
};

class CachePingPongNullLock {
private:
  int rwl;
  CachePingPongNullLock(const CachePingPongNullLock&); //=delete
  void operator=(const CachePingPongNullLock&); //=delete
  //CachePingPongNullLock& operator=(const CachePingPongNullLock&&); // maybe implement
public:
  CachePingPongNullLock(){ unlock(); }
  void rdlock() { asm volatile ("lock decl %1": "+m" (rwl) ::"memory"); }
  void wrlock() { rdlock(); }
  //void unlock() { int v = 1; asm volatile ("xchg %1, %0": "+m" (rwl), "+r" (v)::"memory"); }
  void unlock() { asm("movl $1, %0\n\t": "=m"(rwl)::"memory"); }
};

}; //namespace lintel

#endif // include guard
