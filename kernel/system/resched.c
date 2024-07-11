#include <barelib.h>
#include <syscall.h>
#include <thread.h>
#include <bareio.h>
#include <queue.h>

/*  'resched' places the current running thread into the ready state  *
 *  and  places it onto  the tail of the  ready queue.  Then it gets  *
 *  the head  of the ready  queue  and sets this  new thread  as the  *
 *  'current_thread'.  Finally,  'resched' uses 'ctxsw' to swap from  *
 *  the old thread to the new thread.                                 */
int32 resched(void) {
  uint64 old_threadid = current_thread;
  uint64 new_threadid = thread_dequeue(NTHREADS);
  if(new_threadid == NTHREADS) {
    return current_thread;
  }
  if(thread_table[current_thread].state == TH_RUNNING || thread_table[current_thread].state == TH_READY)
  {
    thread_table[current_thread].state = TH_READY;
    thread_enqueue(NTHREADS, current_thread);
  }
  thread_table[new_threadid].state = TH_RUNNING;
  current_thread = new_threadid;
  ctxsw(&thread_table[new_threadid].stackptr,&thread_table[old_threadid].stackptr);
  return new_threadid;
}
