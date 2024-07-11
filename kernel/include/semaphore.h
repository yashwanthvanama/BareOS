#ifndef H_SEM
#define H_SEM

#include <thread.h>

#define S_FREE 0  /*  Macros for indicating if a semaphore entry is  */
#define S_USED 1  /*  free or currently in use.                      */

#define NSEM NTHREADS + 20                 /*  Number of semaphores                                 */

/*  Each semaphore's state is stored in a 'semaphore_t' structure found in the sem_table  *
 *    (see system/semaphore.c)                                                            */
typedef struct _sem {
  char state;           /*  The current state of the semaphore (S_FREE or S_USED)                          */
  int32 count;          /*  The number of signals sent to this semaphore (negative if threads are waiting  */
  uint32 qprev;         /*  The next element in the queue                                                  */
  uint32 qnext;         /*  The previous element in the queue                                              */
} semaphore_t;

extern semaphore_t sem_table[];

/*  Semaphore related prototypes */
int32 sem_create(int32);
int32 sem_free(uint32);
int32 sem_wait(uint32);
int32 sem_post(uint32);

#endif
