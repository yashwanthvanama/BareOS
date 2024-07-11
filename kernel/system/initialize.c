#include <barelib.h>
#include <interrupts.h>
#include <bareio.h>
#include <shell.h>
#include <thread.h>
#include <queue.h>
#include <semaphore.h>
#include <malloc.h>
#include <tty.h>
#include <string.h>
#include <fs.h>

/*
 *  This file contains the C code entry point executed by the kernel.
 *  It is called by the bootstrap sequence once the hardware is configured.
 *      (see bootstrap.s)
 */

extern uint32* text_start;    /*                                             */
extern uint32* data_start;    /*  These variables automatically contain the  */
extern uint32* bss_start;     /*  lowest  address of important  segments in  */
extern uint32* mem_start;     /*  memory  created  when  the  kernel  boots  */
extern uint32* mem_end;       /*    (see mmap.c and kernel.ld for source)    */

void ctxload(uint64**);

uint32 boot_complete = 0;

void initialize(void) {
  char mask;
  mask = disable_interrupts();
  uart_init();
  restore_interrupts(mask);
  boot_complete = 1;
  printf("Kernel start: %x\n",(unsigned long) text_start);
  printf("--Kernel size: %d\n",(unsigned long) (data_start - text_start));
  printf("Globals start: %x\n",(unsigned long) data_start);
  printf("Heap/Stack start: %x\n",(unsigned long) mem_start);
  printf("--Free Memory Available: %d\n",(unsigned long) (mem_end - mem_start));
  for(int i = 0; i<NTHREADS; i++) {
    thread_table[i].state = TH_FREE;
    thread_queue[i].key = 0;
    thread_queue[i].qnext = i;
    thread_queue[i].qprev = i;
  }
  thread_queue[NTHREADS].key = 0;
  thread_queue[NTHREADS].qprev = NTHREADS;
  thread_queue[NTHREADS].qnext = NTHREADS;
  thread_queue[NTHREADS + 1].key = 0;
  thread_queue[NTHREADS + 1].qprev = NTHREADS + 1;
  thread_queue[NTHREADS + 1].qnext = NTHREADS + 1;
  for(int i=0; i<NSEM; i++) {
    sem_table[i].state = S_FREE;
  }
  heap_init();
  tty_init();
  bs_mk_ramdisk(MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS);
  fs_mkfs();
  fs_mount();
  int32 threadid = create_thread((void (*)(void))shell, "\0", 0);
  thread_table[threadid].state = TH_RUNNING;
  ctxload(&thread_table[threadid].stackptr);
}
