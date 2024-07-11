/*
 *  This file contains functions for initializing and handling interrupts
 *  from the hardware timer.
 */

#include <barelib.h>
#include <interrupts.h>
#include <thread.h>
#include <queue.h>

#define TRAP_TIMER_ENABLE 0x80

volatile uint32* clint_timer_addr  = (uint32*)0x2004000;
const uint32 timer_interval = 100000;
int64 resched(void);

/*
 * This function is called as part of the bootstrapping sequence
 * to enable the timer. (see bootstrap.s)
 */
void clk_init(void) {
  *clint_timer_addr = timer_interval;
  set_interrupt(TRAP_TIMER_ENABLE);
}

/* 
 * This function is triggered every 'timer_interval' microseconds 
 * automatically.  (see '__traps' in bootstrap.s)
 */
interrupt handle_clk(void) {
  uint32 threadid;
  *clint_timer_addr += timer_interval;
  if (boot_complete && is_interrupting()) {
    char mask = disable_interrupts();
    thread_queue[thread_queue[NTHREADS + 1].qnext].key--;
    while(thread_queue[thread_queue[NTHREADS + 1].qnext].key == 0 && thread_queue[NTHREADS + 1].qnext != NTHREADS + 1) {
      threadid = thread_dequeue(NTHREADS + 1);
      thread_table[threadid].state = TH_READY;
      thread_enqueue(NTHREADS, threadid);
    }
    resched();
    restore_interrupts(mask);
  }
}
