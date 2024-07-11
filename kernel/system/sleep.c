#include <interrupts.h>
#include <queue.h>
#include <syscall.h>

/*  Places the thread into a sleep state and inserts it into the  *
 *  sleep delta list.                                             */
int32 sleep(uint32 threadid, uint32 delay) {
  char mask;
  mask = disable_interrupts();
  if(delay == 0) {
    raise_syscall(RESCHED);
    return 0;
  }
  thread_specific_dequeue(NTHREADS, threadid);
  thread_table[threadid].priority = delay;
  thread_enqueue(NTHREADS + 1, threadid);
  thread_table[threadid].state = TH_SLEEP;
  raise_syscall(RESCHED);
  restore_interrupts(mask);
  return 0;
}

/*  If the thread is in the sleep state, remove the thread from the  *
 *  sleep queue and resumes it.                                      */
int32 unsleep(uint32 threadid) {
  char mask;
  mask = disable_interrupts();
  if(thread_table[threadid].state != TH_SLEEP)
    return -1;
  thread_queue[thread_queue[threadid].qnext].key = thread_queue[thread_queue[threadid].qnext].key + thread_queue[threadid].key;
  thread_specific_dequeue(NTHREADS + 1, threadid);
  thread_enqueue(NTHREADS, threadid);
  thread_table[threadid].state = TH_READY;
  raise_syscall(RESCHED);
  restore_interrupts(mask);
  return 0;
}
