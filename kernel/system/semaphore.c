#include <interrupts.h>
#include <queue.h>
#include <semaphore.h>
#include <thread.h>
#include <syscall.h>

semaphore_t sem_table[NSEM];
void sem_enqueue(uint32, uint32);
uint32 sem_dequeue(uint32);
uint32 find_threadid(uint32);

/*  Finds an unusued semaphore in the sem_table and returns it after  *
 *  resetting its values and marking it as used.                      */
int32 sem_create(int32 count) {
  int i;
  char mask = disable_interrupts();
  for(i = 0; i < NSEM; i++) {
    if(sem_table[i].state == S_FREE) {
      break;
    }
  }
  if(i != NSEM) {
    sem_table[i].state = S_USED;
    sem_table[i].count = count;
    sem_table[i].qprev = i;
    sem_table[i].qnext = i;
    restore_interrupts(mask);
    return i;
  }
  else {
    restore_interrupts(mask);
    return -1;
  }
}

/*  Marks a semaphore as free  */
int32 sem_free(uint32 sid) {
  if(sem_table[sid].state == S_FREE) {
    return -1;
  }
  else {
    uint32 i = sem_table[sid].qnext;
    do {
      if(i == sid) {
        break;
      }
      uint32 threadid = find_threadid(i);
      if(threadid == -1){
        i = sem_table[i].qnext;
        continue;
      }
      thread_enqueue(NTHREADS, threadid);
      thread_table[threadid].state = TH_READY;
      i = sem_table[i].qnext;
    } while(i != sid);
    sem_table[sid].state = S_FREE;
    raise_syscall(RESCHED);
    return 0;
  }
}

/*  Decrements the given semaphore if it is in use.  If the semaphore  *
 *  is less than 0, marks the thread as waiting and switches to        *
 *  another thread.                                                    */
int32 sem_wait(uint32 sid) {
  char mask;
  mask = disable_interrupts();
  if(sid < 0 || sid >= NSEM || sem_table[sid].state == S_FREE) {
    restore_interrupts(mask);
    return -1;
  }
  else {
    sem_table[sid].count--;
    if(sem_table[sid].count < 0 && sem_table[sid].state == S_USED) {
      sem_enqueue(sid, thread_table[current_thread].semid);
      thread_table[current_thread].state = TH_WAITING;
      raise_syscall(RESCHED);
    }
  }
  restore_interrupts(mask);
  return 0;
}

/*  Increments the given semaphore if it is in use.  Resume the next  *
 *  waiting thread (if any).                                          */
int32 sem_post(uint32 sid) {
  char mask;
  mask = disable_interrupts();
  if(sid < 0 || sid >= NSEM || sem_table[sid].state == S_FREE) {
    restore_interrupts(mask);
    return -1;
  }
  else if(sem_table[sid].count < 0){
    sem_table[sid].count++;
    uint32 threadid = find_threadid(sem_dequeue(sid));
    if(threadid == -1)
      return 0;
    thread_enqueue(NTHREADS, threadid);
    thread_table[threadid].state = TH_READY;
    raise_syscall(RESCHED);
  }
  restore_interrupts(mask);
  return 0;
}

void sem_enqueue(uint32 main_sem, uint32 wait_sem) {
  uint32 i = sem_table[main_sem].qnext;
  do {
    if(i == wait_sem) {
      return;
    }
    i = sem_table[i].qnext;
  } while(i != main_sem);
  i = sem_table[main_sem].qprev;
  sem_table[i].qnext = wait_sem;
  sem_table[wait_sem].qprev = i;
  sem_table[wait_sem].qnext = main_sem;
  sem_table[main_sem].qprev = wait_sem;
  return;
}

uint32 sem_dequeue(uint32 semid) {
  if(sem_table[semid].qnext == semid) {
    return semid;
  }
  else {
    uint32 i = sem_table[semid].qnext;
    sem_table[semid].qnext = sem_table[i].qnext;
    sem_table[sem_table[i].qnext].qprev = semid;
    return i;
  }
}

uint32 find_threadid(uint32 semid) {
  for(int i = 0; i < NTHREADS; i++) {
    if(thread_table[i].semid == semid)
      return i;
  }
  return -1;
}