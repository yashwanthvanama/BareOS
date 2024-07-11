#include <barelib.h>
#include <thread.h>
#include <interrupts.h>
#include <syscall.h>
#include <semaphore.h>


/*  Takes an index into the thread_table.  If the thread is TH_DEFUNCT,  *
 *  mark  the thread  as TH_FREE  and return its  `retval`.   Otherwise  *
 *  raise RESCHED and loop to check again later.                         */
byte join_thread(uint32 threadid) {
  while(thread_table[threadid].state != TH_DEFUNCT && thread_table[threadid].state != TH_FREE)  {
    raise_syscall(RESCHED);
  }
  thread_table[threadid].state = TH_FREE;
  return thread_table[threadid].retval;
}
