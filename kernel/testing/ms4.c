#include <barelib.h>
#include <syscall.h>
#include <queue.h>
#include <interrupts.h>
#include "tests.h"

extern byte t__default_resched;
extern byte t__auto_timeout;
extern int32 t__timeout;


static const char* general_prompt[] = {
			     "  Program Compiles:             "
};
static const char* initialization_prompt[] = {
			     "    List initialization:        ",
};
static const char* enqueue_prompt[] = {
			     "    List integrity [1-thread]:  ",
			     "    List integrity [2-threads]: ",
			     "    List integrity [3-threads]: ",
};
static const char* dequeue_prompt[] = {
			     "    First pop:                  ",
			     "    First return:               ",
			     "    Second pop:                 ",
			     "    Second return:              ",
			     "    Third pop:                  ",
			     "    Third return:               ",
};
static const char* order_prompt[] = {
			     "    Enqueue first:              ",
			     "    Enqueue second:             ",
			     "    Enqueue third:              ",
			     "    Dequeue first:              ",
			     "    Dequeue second:             ",
			     "    Dequeue third:              ",
};
static const char* resched_prompt[] = {
			     "  Enqueue on resume:            ",
			     "  Dequeue on suspend:           ",
			     "  Resched dequeues/enqueues:    ",
};
static char* general_t[test_count(general_prompt)];
static char* initialization_t[test_count(initialization_prompt)];
static char* enqueue_t[test_count(enqueue_prompt)];
static char* dequeue_t[test_count(dequeue_prompt)];
static char* order_t[test_count(order_prompt)];
static char* resched_t[test_count(resched_prompt)];

static byte dummy_thread(char* arg) {
  while (1)
    raise_syscall(RESCHED);
  return 0;
}

static byte resched_thread(char* arg) {
  assert(thread_queue[ready_list].qnext == 0, resched_t[2], "FAIL - 'ready_list.qnext' does not point to shell after resched");
  assert(thread_queue[ready_list].qprev == 0, resched_t[2], "FAIL - 'ready_list.qprev' does not point to shell after resched");
  return 0;
}

static byte join_thread_local(uint32 threadid) {
  char mask = disable_interrupts();
  byte result;

  if (thread_table[threadid].state == TH_FREE) {
    restore_interrupts(mask);
    return NTHREADS;
  }
  
  while (thread_table[threadid].state != TH_DEFUNCT && t__timeout > 0) {
    raise_syscall(RESCHED);
  }
  thread_table[threadid].state = TH_FREE;
  result = thread_table[threadid].retval;

  restore_interrupts(mask);
  return result;
}

static void general_tests(void) {
  return;
}

static void initialization_tests(void) {
  assert(thread_queue[ready_list].qnext == ready_list, initialization_t[0], "FAIL - 'ready_list.qnext' does not point to itself");
  assert(thread_queue[ready_list].qprev == ready_list, initialization_t[0], "FAIL - 'ready_list.qprev' does not point to itself");
  t__mem_restore();
}

static void enqueue_tests(void) {
  int32 tid1, tid2, tid3;
  tid1 = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  return_on_timeout(ALL, enqueue);
  
  t__with_timeout(10, thread_enqueue, ready_list, tid1);
  maybe_timeout(0, enqueue) {
    assert(thread_queue[ready_list].qnext == tid1, enqueue_t[0], "FAIL - 'ready_list.qnext' does not point to new thread");
    assert(thread_queue[ready_list].qprev == tid1, enqueue_t[0], "FAIL - 'ready_list.qprev' does not point to new thread");
    assert(thread_queue[tid1].qprev == ready_list, enqueue_t[0], "FAIL - new thread's 'qprev' does not point to 'ready_list'");
    assert(thread_queue[tid1].qnext == ready_list, enqueue_t[0], "FAIL - new thread's 'qnext' does not point to 'ready_list'");
  }

  tid2 = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, thread_enqueue, ready_list, tid2);
  
  maybe_timeout(1, enqueue) {
    assert(thread_queue[ready_list].qnext == tid1, enqueue_t[1], "FAIL - 'ready_list.qnext' does not point to first thread");
    assert(thread_queue[ready_list].qprev == tid2, enqueue_t[1], "FAIL - 'ready_list.qprev' does not point to new thread");
    assert(thread_queue[tid1].qnext == tid2, enqueue_t[1], "FAIL - first thread's 'qnext' does not point to new thread");
    assert(thread_queue[tid1].qprev == ready_list, enqueue_t[1], "FAIL - first thread's 'qprev' does not point to 'ready_list'");
    assert(thread_queue[tid2].qnext == ready_list, enqueue_t[1], "FAIL - new thread's 'qnext' does not point to 'ready_list'");
    assert(thread_queue[tid2].qprev == tid1, enqueue_t[1], "FAIL - new thread's 'qprev' does not point to first thread");
  }
  tid3 = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, thread_enqueue, ready_list, tid3);
  maybe_timeout(2, enqueue) {
    assert(thread_queue[ready_list].qnext == tid1, enqueue_t[2], "FAIL - 'ready_list.qnext' does not point to first thread");
    assert(thread_queue[ready_list].qprev == tid3, enqueue_t[2], "FAIL - 'ready_list.qprev' does not point to new thread");
    assert(thread_queue[tid1].qnext == tid2, enqueue_t[2], "FAIL - first thread's 'qnext' does not point to second thread");
    assert(thread_queue[tid1].qprev == ready_list, enqueue_t[2], "FAIL - first thread's 'qprev' does not point to 'ready_list'");
    assert(thread_queue[tid2].qnext == tid3, enqueue_t[2], "FAIL - second thread's 'qnext' does not point to new thread");
    assert(thread_queue[tid2].qprev == tid1, enqueue_t[2], "FAIL - second thread's 'qprev' does not point to first thread");
    assert(thread_queue[tid3].qnext == ready_list, enqueue_t[2], "FAIL - new thread's 'qnext' does not point to 'ready_list'");
    assert(thread_queue[tid3].qprev == tid2, enqueue_t[2], "FAIL - new thread's 'qprev' does not point to second thread");
  }
  t__mem_restore();
}

static void dequeue_setup(int32* tid1, int32* tid2, int32* tid3) {
  *tid1 = create_thread(dummy_thread, "", 0);
  *tid2 = create_thread(dummy_thread, "", 0);
  *tid3 = create_thread(dummy_thread, "", 0);

  thread_enqueue(ready_list, *tid1);
  thread_enqueue(ready_list, *tid2);
  thread_enqueue(ready_list, *tid3);
}

static void dequeue_tests(void) {
  int32 tid1, tid2, tid3;
  int32 ret1, ret2, ret3;
  t__with_timeout(10, dequeue_setup, &tid1, &tid2, &tid3);
  return_on_timeout(ALL, dequeue);

  ret1 = t__with_timeout(10, thread_dequeue, ready_list);
  maybe_timeout(0, dequeue) {
    assert(thread_queue[ready_list].qnext == tid2, dequeue_t[0], "FAIL - 'ready_list.qnext' does not point to second thread");
    assert(thread_queue[ready_list].qprev == tid3, dequeue_t[0], "FAIL - 'ready_list.qprev' does not point to third thread");
    assert(thread_queue[tid2].qnext == tid3, dequeue_t[0], "FAIL - second thread's 'qnext' does not point to third thread");
    assert(thread_queue[tid2].qprev == ready_list, dequeue_t[0], "FAIL - second thread's 'qprev' does not point to 'ready_list'");
    assert(thread_queue[tid3].qnext == ready_list, dequeue_t[0], "FAIL - third thread's 'qnext' does not point to 'ready_list'");
    assert(thread_queue[tid3].qprev == tid2, dequeue_t[0], "FAIL - third thread's 'qprev' does not point to first thread");
  }
  maybe_timeout(1, dequeue) {
    assert(ret1 == tid1, dequeue_t[1], "FAIL - return value is not the first enqueued thread'");
  }
  
  ret2 = t__with_timeout(10, thread_dequeue, ready_list);
  maybe_timeout(2, dequeue) {
    assert(thread_queue[ready_list].qnext == tid3, dequeue_t[2], "FAIL - 'ready_list.qnext' does not point to third thread");
    assert(thread_queue[ready_list].qprev == tid3, dequeue_t[2], "FAIL - 'ready_list.qprev' does not point to third thread");
    assert(thread_queue[tid3].qnext == ready_list, dequeue_t[2], "FAIL - third thread's 'qnext' does not point to 'ready_list'");
    assert(thread_queue[tid3].qprev == ready_list, dequeue_t[2], "FAIL - third thread's 'qprev' does not point to 'ready_list'");
  }
  maybe_timeout(3, dequeue) {
    assert(ret2 = tid2, dequeue_t[3], "FAIL - return value is not the second enqueued thread'");
  }
  
  ret3 = t__with_timeout(10, thread_dequeue, ready_list);
  maybe_timeout(4, dequeue) {
    assert(thread_queue[ready_list].qnext == ready_list, dequeue_t[4], "FAIL - 'ready_list.qnext' does not point to 'ready_list'");
    assert(thread_queue[ready_list].qprev == ready_list, dequeue_t[4], "FAIL - 'ready_list.qprev' does not point to 'ready_list'");
  }
  maybe_timeout(5, dequeue) {
    assert(ret3 = tid3, dequeue_t[5], "FAIL - return value is not the third enqueued thread'");
  }

  t__mem_restore();
}

static void order_setup(int32* tid1, int32* tid2, int32* tid3) {
  *tid1 = create_thread(dummy_thread, "", 0);
  *tid2 = create_thread(dummy_thread, "", 0);
  *tid3 = create_thread(dummy_thread, "", 0);
}

static void order_tests(void) {
  int32 tid1, tid2, tid3;
  t__with_timeout(10, order_setup, &tid1, &tid2, &tid3);
  return_on_timeout(ALL, order);
  
  t__with_timeout(10, thread_enqueue, ready_list, tid3);
  maybe_timeout(0, order) {
    assert(thread_queue[ready_list].qnext == tid3, order_t[0], "FAIL - 'ready_list.qnext' does not point to out-of-order entry 1");
    assert(thread_queue[ready_list].qprev == tid3, order_t[0], "FAIL - 'ready_list.qprev' does not point to out-of-order entry 1");
  }
  
  t__with_timeout(10, thread_enqueue, ready_list, tid1);
  maybe_timeout(1, order) {
    assert(thread_queue[ready_list].qnext == tid3, order_t[1], "FAIL - 'ready_list.qnext' does not point to out-of-order entry 1");
    assert(thread_queue[ready_list].qprev == tid1, order_t[1], "FAIL - 'ready_list.qprev' does not point to out-of-order entry 2");
    assert(thread_queue[tid3].qnext == tid1,       order_t[1], "FAIL - out-of-order entry 1 'qnext' does not point to out-of-order entry 2");
    assert(thread_queue[tid3].qprev == ready_list, order_t[1], "FAIL - out-of-order entry 1 'qprev' does not point to 'ready_list'");
    assert(thread_queue[tid1].qnext == ready_list, order_t[1], "FAIL - out-of-order entry 2 'qnext' does not point to 'ready_list'");
    assert(thread_queue[tid1].qprev == tid3,       order_t[1], "FAIL - out-of-order entry 2 'qprev' does not point to out-of-order entry 1");
  }

  t__with_timeout(10, thread_enqueue, ready_list, tid2);
  maybe_timeout(2, order) {
    assert(thread_queue[ready_list].qnext == tid3, order_t[2], "FAIL - 'ready_list.qnext' does not point to out-of-order entry 1");
    assert(thread_queue[ready_list].qprev == tid2, order_t[2], "FAIL - 'ready_list.qprev' does not point to out-of-order entry 3");
    assert(thread_queue[tid3].qnext == tid1,       order_t[2], "FAIL - out-of-order entry 1 'qnext' does not point to out-of-order entry 2");
    assert(thread_queue[tid3].qprev == ready_list, order_t[2], "FAIL - out-of-order entry 1 'qprev' does not point to 'ready_list'");
    assert(thread_queue[tid1].qnext == tid2,       order_t[2], "FAIL - out-of-order entry 2 'qnext' does not point to out-of-order entry 3");
    assert(thread_queue[tid1].qprev == tid3,       order_t[2], "FAIL - out-of-order entry 2 'qprev' does not point to out-of-order entry 1");
    assert(thread_queue[tid2].qnext == ready_list, order_t[2], "FAIL - out-of-order entry 3 'qnext' does not point to 'ready_list'");
    assert(thread_queue[tid2].qprev == tid1,       order_t[2], "FAIL - out-of-order entry 3 'qprev' does not point to out-of-order entry 2");
  }
  
  t__with_timeout(10, thread_dequeue, ready_list);
  maybe_timeout(3, order) {
    assert(thread_queue[ready_list].qnext == tid1, order_t[3], "FAIL - 'ready_list.qnext' does not point to out-of-order entry 2");
    assert(thread_queue[ready_list].qprev == tid2, order_t[3], "FAIL - 'ready_list.qprev' does not point to out-of-order entry 3");
    assert(thread_queue[tid1].qnext == tid2,       order_t[3], "FAIL - out-of-order entry 2 'qnext' does not point to out-of-order entry 3");
    assert(thread_queue[tid1].qprev == ready_list, order_t[3], "FAIL - out-of-order entry 2 'qprev' does not point to 'ready_list'");
    assert(thread_queue[tid2].qnext == ready_list, order_t[3], "FAIL - out-of-order entry 3 'qnext' does not point to 'ready_list'");
    assert(thread_queue[tid2].qprev == tid1,       order_t[3], "FAIL - out-of-order entry 3 'qprev' does not point to out-of-order entry 2");
  }

  t__with_timeout(10, thread_dequeue, ready_list);
  maybe_timeout(4, order) {
    assert(thread_queue[ready_list].qnext == tid2, order_t[4], "FAIL - 'ready_list.qnext' does not point to out-of-order entry 3");
    assert(thread_queue[ready_list].qprev == tid2, order_t[4], "FAIL - 'ready_list.qprev' does not point to out-of-order entry 3");
    assert(thread_queue[tid2].qnext == ready_list, order_t[4], "FAIL - out-of-order entry 3 'qnext' does not point to 'ready_list'");
    assert(thread_queue[tid2].qprev == ready_list, order_t[4], "FAIL - out-of-order entry 3 'qprev' does not point to 'ready_list'");
  }
  
  t__with_timeout(10, thread_dequeue, ready_list);
  maybe_timeout(5, order) {
    assert(thread_queue[ready_list].qnext == ready_list, order_t[5], "FAIL - 'ready_list.qnext' does not point to 'ready_list'");
    assert(thread_queue[ready_list].qprev == ready_list, order_t[5], "FAIL - 'ready_list.qprev does not point to 'ready_list'");
  }
  t__mem_restore();
}

static void resched_tests(void) {
  int32 tid;
  t__auto_timeout = 1;
  tid = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  return_on_timeout(ALL, resched);

  t__with_timeout(10, (void (*)(void))resume_thread, tid);
  maybe_timeout(0, resched) {
    assert(thread_queue[ready_list].qnext == tid, resched_t[0], "FAIL - 'ready_list.qnext' does not point to new thread");
    assert(thread_queue[ready_list].qprev == tid, resched_t[0], "FAIL - 'ready_list.qprev' does not point to new thread");
  }

  t__with_timeout(10, (void (*)(void))suspend_thread, tid);
  maybe_timeout(1, resched) {
    assert(thread_queue[ready_list].qnext == ready_list, resched_t[1], "FAIL - 'ready_list.qnext' does not point to 'ready_list'");
    assert(thread_queue[ready_list].qprev == ready_list, resched_t[1], "FAIL - 'ready_list.qnext' does not point to 'ready_list'");
  }

  t__mem_restore();
  
  tid = t__with_timeout(10, create_thread, resched_thread, "", 0);
  return_on_timeout(2, resched);
  t__default_resched = 1;
  t__with_timeout(10, (void (*)(void))resume_thread, tid);
  return_on_timeout(2, resched);
  
  t__auto_timeout = 0;
  t__timeout = 10;
  join_thread_local(tid);
  assert(t__timeout > 0, resched_t[2], "FAIL - Timeout after resuming queued thread");
}

void t__ms4(uint32 idx) {
  t__auto_timeout = 0;
  t__mem_snapshot();
  if (idx == 0) {
    t__print("\n");
    runner("general", general);
  }
  else if (idx == 1) {
    runner("initialization", initialization);
  }
  else if (idx == 2) {
    runner("enqueue", enqueue);
  }
  else if (idx == 3) {
    runner("dequeue", dequeue);
  }
  else if (idx == 4) {
    runner("order", order);
  }
  else if (idx == 5) {
    runner("resched", resched);
  }
  else {
    t__print("\n----------------------------\n");
    feedback("General", general);
    t__print("\nQueue Tests:");
    feedback("  Initialization", initialization);
    feedback("  Enqueue", enqueue);
    feedback("  Dequeue", dequeue);
    feedback("  Out of Order", order);
    t__print("\n");
    feedback("Scheduling", resched);
    t__print("\n");
  }
}
