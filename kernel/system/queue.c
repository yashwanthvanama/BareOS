#include <queue.h>
#include <bareio.h>
#include <barelib.h>

/*  Queues in bareOS are all contained in the 'thread_queue' array.  Each queue has a "root"
 *  that contains the index of the first and last elements in that respective queue.  These
 *  roots are  found at the end  of the 'thread_queue'  array.  Following the 'qnext' index 
 *  of each element, starting at the "root" should always eventually lead back to the "root".
 *  The same should be true in reverse using 'qprev'.                                          */

queue_t thread_queue[NTHREADS + 2];   /*  Array of queue elements, one per thread plus one for the read_queue root  */
uint32 ready_list = NTHREADS + 0;   /*  Index of the read_list root  */
uint32 sleep_list = NTHREADS + 1;
void print_thread_queue();


/*  'thread_enqueue' takes an index into the thread_queue  associated with a queue "root"  *
 *  and a threadid of a thread to add to the queue.  The thread will be added to the tail  *
 *  of the queue,  ensuring that the  previous tail of the queue is correctly threaded to  *
 *  maintain the queue.                                                                    */
void thread_enqueue(uint32 queue, uint32 threadid) {
  uint32 i = thread_queue[queue].qnext;
  do {
    if(i == threadid) {
      return;
    }
    i = thread_queue[i].qnext;
  } while(i != queue);
  if(thread_queue[queue].qnext == queue) {
    thread_queue[queue].qnext = threadid;
    thread_queue[queue].qprev = threadid;
    thread_queue[threadid].qprev = queue;
    thread_queue[threadid].qnext = queue;
    thread_queue[threadid].key = thread_table[threadid].priority;
  }
  else {
    i = thread_queue[queue].qnext;
    if(queue == NTHREADS) {
      while(thread_queue[i].key <= thread_table[threadid].priority) {
        if(thread_queue[i].qnext == queue) {
          break;
        }
        else {
          i = thread_queue[i].qnext;
        }
      }
    }
    else {
      while(thread_queue[i].key <= thread_table[threadid].priority) {
        thread_table[threadid].priority = thread_table[threadid].priority - thread_queue[i].key;
        if(thread_queue[i].qnext == queue) {
            break;
        }
        else {
          i = thread_queue[i].qnext;
        }
      }
      if(thread_queue[i].qnext != queue && thread_queue[i].key > thread_table[threadid].priority) {
        i = thread_queue[i].qprev;
      }
    }
    thread_queue[threadid].qnext = thread_queue[i].qnext;
    thread_queue[threadid].qprev = i;
    thread_queue[thread_queue[i].qnext].qprev = threadid;
    thread_queue[i].qnext = threadid;
    thread_queue[threadid].key = thread_table[threadid].priority;
    if(queue == NTHREADS + 1 && thread_queue[threadid].qnext != NTHREADS + 1)
      thread_queue[thread_queue[threadid].qnext].key = thread_queue[thread_queue[threadid].qnext].key - thread_queue[threadid].key;
  }
}


/*  'thread_dequeue' takes a queue index associated with a queue "root" and removes the  *
 *  thread at the head of the queue and returns the index of that thread, ensuring that  *
 *  the queue  maintains its structure and the head correctly points to the next thread  *
 *  (if any).                                                                            */
uint32 thread_dequeue(uint32 queue) {
  if(thread_queue[queue].qnext == queue) {
    return NTHREADS;
  }
  else {
    uint32 threadid = thread_queue[queue].qnext;
    thread_queue[queue].qnext = thread_queue[threadid].qnext;
    thread_queue[thread_queue[threadid].qnext].qprev = queue;
    thread_queue[threadid].qnext = threadid;
    thread_queue[threadid].qprev = threadid;
    return threadid;
  }
}

void thread_specific_dequeue(uint32 queue, uint32 threadid) {
  uint32 i = thread_queue[queue].qnext;
  while(i != threadid && i != queue) {
    i = thread_queue[i].qnext;
  }
  if (i == queue) {
    return;
  }
  thread_queue[thread_queue[i].qprev].qnext = thread_queue[i].qnext;
  thread_queue[thread_queue[i].qnext].qprev = thread_queue[i].qprev;
}


void print_thread_queue() {
  for(int i = 0; i <= NTHREADS + 1; i++) {
    printf("key : %d, next : %d, prev : %d \n", thread_queue[i].key, thread_queue[i].qnext, thread_queue[i].qprev);
  }
}