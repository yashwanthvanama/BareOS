#include <barelib.h>
#include <thread.h>
#include <queue.h>
#include <sleep.h>
#include "tests.h"

extern uint32 sleep_list;

extern byte t__auto_timeout;
extern byte t__skip_resched;
extern byte t__default_resched;
extern byte t__default_timer;

static const char* general_prompt[] = {
				       "  Program Compiles:                ",
};
static const char* sleep_prompt[] = {
				       "  Sleep queue is initialized:      ",
				       "  Sets thread's state:             ",
				       "  Adds thread to queue:            ",
				       "  Key set to sleep time [first]:   ",
				       "  Thread removed from ready queue: ",
				       "  Longer sleep added in place:     ",
				       "  Longer sleep key is difference:  ",
				       "  Shorter sleep added at head:     ",
				       "  Queue adjusted for new head:     ",
				       "  Intermediate sleep inserted:     ",
				       "  Intermediate sleep adjusts keys: ",
};
static const char* unsleep_prompt[] = {
				       "  Returns -1 when empty:           ",
				       "  Removed from head:               ",
				       "  Adjusts next key [head]:         ",
				       "  Remove from middle:              ",
				       "  Adjusts next key [middle]:       ",
};
static const char* resched_prompt[] = {
				       "  Clock decrements head:           ",
				       "  Zero delay threads removed:      ",
				       "  Removed multiple zero threads:   ",
				       "  Removed threads readied:         ",
};

static char* general_t[test_count(general_prompt)];
static char* sleep_t[test_count(sleep_prompt)];
static char* unsleep_t[test_count(unsleep_prompt)];
static char* resched_t[test_count(resched_prompt)];

static void resume(uint32 tid) {
  thread_table[tid].state = TH_READY;
  thread_enqueue(ready_list, tid);
}

static byte dummy_thread(char* arg) { while(1); return 0; }

static void general_tests(void) { return; }
static void sleep_tests(void) {
  t__skip_resched = 1;
  
  assert(thread_queue[sleep_list].qnext == sleep_list, sleep_t[0], "FAIL - 'sleep_list.qnext' does not point to itself");
  assert(thread_queue[sleep_list].qprev == sleep_list, sleep_t[0], "FAIL - 'sleep_list.qprev' does not point to itself");
  
  int32 tid1 = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, resume, tid1);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, sleep, tid1, 120);    
  maybe_timeout(1, sleep) {
    assert(thread_table[tid1].state != TH_FREE, sleep_t[1], "FAIL - Sleeping thread's state set to FREE");
    assert(thread_table[tid1].state != TH_READY, sleep_t[1], "FAIL - Sleeping thread's state still READY");
    assert(thread_table[tid1].state != TH_RUNNING, sleep_t[1], "FAIL - Sleeping thread's state set to RUNNING");
    assert(thread_table[tid1].state != TH_SUSPEND, sleep_t[1], "FAIL - Sleeping thread's state set to SUSPEND");
    assert(thread_table[tid1].state != TH_DEFUNCT, sleep_t[1], "FAIL - Sleeping thread's state set to DEFUNCT");
  }

  maybe_timeout(2, sleep) {
    assert(thread_queue[sleep_list].qnext == tid1, sleep_t[2], "FAIL - 'sleep_list.qnext' does not point to slept thread");
    assert(thread_queue[sleep_list].qprev == tid1, sleep_t[2], "FAIL - 'sleed_list.qprev' does not point to slept thread");
    assert(thread_queue[tid1].qnext == sleep_list, sleep_t[2], "FAIL - Slept thread's 'qnext' does not point to sleep_list");
    assert(thread_queue[tid1].qprev == sleep_list, sleep_t[2], "FAIL - Slept thread's 'qprev' does not point to the sleep_list");
  }
  maybe_timeout(3, sleep) {
    assert(thread_queue[tid1].key == 120, sleep_t[3], "FAIL - Thread's key does not match sleep delay value");
  }

  maybe_timeout(4, sleep) {
    assert(thread_queue[ready_list].qnext != tid1, sleep_t[4], "FAIL - Thread still listed in ready_list");
    assert(thread_queue[ready_list].qprev != tid1, sleep_t[4], "FAIL - Thread still listed in ready_list");
  }

  int32 tid2 = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, resume, tid2);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, sleep, tid2, 140);
  maybe_timeout(5, sleep) {
    assert(thread_queue[sleep_list].qnext == tid1, sleep_t[5], "FAIL - Sleepier thread inserted before dozing thread");
    assert(thread_queue[tid1].qprev == sleep_list, sleep_t[5], "FAIL - Sleepier thread inserted before dozing thread");
    assert(thread_queue[tid1].qnext == tid2, sleep_t[5], "FAIL - Second thread not found in sleep queue");
    assert(thread_queue[sleep_list].qprev == tid2, sleep_t[5], "FAIL - Second thread not found in sleep queue");
    assert(thread_queue[tid2].qnext == sleep_list, sleep_t[5], "FAIL - Second thread not found in sleep queue");
    assert(thread_queue[tid2].qprev == tid1, sleep_t[5], "FAIL - Second thread not found in sleep queue");
  }
  maybe_timeout(6, sleep) {
    assert(thread_queue[tid2].key == 20, sleep_t[6], "FAIL - Added thread's key is not the remaining sleep time");
    assert(thread_queue[tid2].key != 120, sleep_t[6], "FAIL - Added thread's key not adjusted by existing keys");
    assert(thread_queue[tid1].key == 120, sleep_t[6], "FAIL - Sleep adjusted the delay of a preceeding thread");
  }

  int32 tid3 = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, resume, tid3);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, sleep, tid3, 88);

  maybe_timeout(7, sleep) {
    assert(thread_queue[sleep_list].qnext == tid3, sleep_t[7], "FAIL - Short sleep not placed at head of list");
    assert(thread_queue[tid1].qprev == tid3, sleep_t[7], "FAIL - Short sleep not placed at head of list");
    assert(thread_queue[tid1].qnext == tid2, sleep_t[7], "FAIL - Inserting short sleep broke queue threading");
    assert(thread_queue[tid2].qprev == tid1, sleep_t[7], "FAIL - Inserting short sleep broke queue threading");
    assert(thread_queue[tid2].qnext == sleep_list, sleep_t[7], "FAIL - Inserting short sleep broke queue threading");
    assert(thread_queue[tid3].qnext == tid1, sleep_t[7], "FAIL - New thread not pointing to previous head");
    assert(thread_queue[tid3].qprev == sleep_list, sleep_t[7], "FAIL - New thread does not point back to sleep_list");
  }
  maybe_timeout(8, sleep) {
    assert(thread_queue[tid3].key == 88, sleep_t[8], "FAIL - New thread's key is not set to sleep duration");
    assert(thread_queue[tid1].key == 32, sleep_t[8], "FAIL - Next thread's key was not adjusted on insert");
    assert(thread_queue[tid2].key == 20, sleep_t[8], "FAIL - Sleep adjusted the wrong thread's delay value");
  }

  int32 tid4 = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, resume, tid4);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, sleep, tid4, 100);

  maybe_timeout(9, sleep) {
    assert(thread_queue[tid3].qnext == tid4, sleep_t[9], "FAIL - Thread was not inserted in the correct position");
    assert(thread_queue[tid1].qprev == tid4, sleep_t[9], "FAIL - Thread was not inserted in the correct position");
    assert(thread_queue[tid4].qnext == tid1, sleep_t[9], "FAIL - Thread was not inserted in the correct position");
    assert(thread_queue[tid4].qprev == tid3, sleep_t[9], "FAIL - Thread was not inserted in the correct position");
  }
  maybe_timeout(10, sleep) {
    assert(thread_queue[tid3].key == 88, sleep_t[10], "FAIL - Key of previous thread was adjusted");
    assert(thread_queue[tid4].key == 12, sleep_t[10], "FAIL - Key was not adjusted for previous delays");
    assert(thread_queue[tid1].key == 20, sleep_t[10], "FAIL - Key of the next thread was not adjusted");
  }
}

static void unsleep_setup(int32* tid1, int32* tid2, int32* tid3, int32* tid4) {
  *tid1 = create_thread(dummy_thread, "", 0);
  *tid2 = create_thread(dummy_thread, "", 0);
  *tid3 = create_thread(dummy_thread, "", 0);
  *tid4 = create_thread(dummy_thread, "", 0);
  resume(*tid1);
  resume(*tid2);
  resume(*tid3);
  resume(*tid4);
}

static void unsleep_sleep(int32 tid1, int32 tid2, int32 tid3, int32 tid4) {
  sleep(tid1, 5);
  sleep(tid2, 10);
  sleep(tid3, 15);
  sleep(tid4, 20);
  unsleep(tid1);
}

static void unsleep_tests(void) {
  t__skip_resched = 1;
  t__default_resched = 1;

  int32 result = t__with_timeout(10, unsleep, 1);

  maybe_timeout(0, unsleep) {
    assert(result == -1, unsleep_t[0], "FAIL - Did not return -1 when emtpy");
  }

  t__mem_restore();
  int32 tid1, tid2, tid3, tid4;
  t__with_timeout(10, unsleep_setup, &tid1, &tid2, &tid3, &tid4);
  return_on_timeout(ALL, unsleep);

  t__with_timeout(10, unsleep_sleep, tid1, tid2, tid3, tid4);
  assert(thread_queue[sleep_list].qnext == tid2, unsleep_t[1], "FAIL - Unexpected thread at head of sleep_list");
  assert(thread_queue[tid2].qprev == sleep_list, unsleep_t[1], "FAIL - Unexpected thread at head of sleep_list");
  assert(thread_queue[tid2].key == 10, unsleep_t[2], "FAIL - Key for new head was not expected value");
  assert(thread_queue[tid2].key != 5, unsleep_t[2], "FAIL - Key for new head was not adjusted");
  maybe_timeout(1, unsleep) {}
  maybe_timeout(2, unsleep) {}

  t__with_timeout(10, unsleep, tid3);
  assert(thread_queue[sleep_list].qnext == tid2, unsleep_t[3], "FAIL - remove from middle of list changed head");
  assert(thread_queue[tid2].qnext == tid4, unsleep_t[3], "FAIL - Threading of list wrong after removal");
  assert(thread_queue[tid2].qnext != tid3, unsleep_t[3], "FAIL - Removed thread still in list");
  assert(thread_queue[tid2].key == 10, unsleep_t[4], "FAIL - Head's key value changed");
  assert(thread_queue[tid4].key == 10, unsleep_t[4], "FAIL - Key for tail was not expected value");
  assert(thread_queue[tid4].key != 5, unsleep_t[4], "FAIL - Key for tail was not adjusted");
  maybe_timeout(3, unsleep) {}
  maybe_timeout(4, unsleep) {}
}

static void wait_for_tick(int32 tid, int32 target) {
  while (thread_queue[tid].key > target)
    continue;
}

static void resched_tests(void) {
  t__skip_resched = 1;
  t__default_timer = 1;
  int32 tid, tid2;
  tid = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, resume, tid);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, sleep, tid, 10);
  if (!status_is(TIMEOUT))
    t__with_timeout(20, wait_for_tick, tid, 9);

  maybe_timeout(0, resched) {}

  t__mem_restore();

  tid = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, resume, tid);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, sleep, tid, 10);
  if (!status_is(TIMEOUT))
    t__with_timeout(20, wait_for_tick, tid, 0);

  assert(thread_queue[sleep_list].qnext == sleep_list, resched_t[1], "FAIL - Sleep list still contains thread");
  assert(thread_table[tid].state == TH_READY, resched_t[3], "FAIL - Thread with 0 delay was not readied");
  maybe_timeout(1, resched) {}
  maybe_timeout(3, resched) {}

  t__mem_restore();
  tid = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  if (!status_is(TIMEOUT))
    tid2 = t__with_timeout(10, create_thread, dummy_thread, "", 0);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, resume, tid);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, resume, tid2);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, sleep, tid, 1);
  if (!status_is(TIMEOUT))
    t__with_timeout(10, sleep, tid2, 1);
  int32 ttid = (thread_queue[sleep_list].qnext == tid ? tid : tid2);
  if (!status_is(TIMEOUT))
    t__with_timeout(20, wait_for_tick, ttid, 0);
  maybe_timeout(2, resched) {
    assert(thread_queue[sleep_list].qnext == sleep_list, resched_t[2], "FAIL - Sleep list still contains threads");
  }
  t__default_timer = 0;
}

void t__ms5(uint32 idx) {
  t__mem_snapshot();
  if (idx == 0) {
    t__print("\n");
    runner("general", general);
  }
  else if (idx == 1) {
    runner("sleep", sleep);
  }
  else if (idx == 2) {
    runner("unsleep", unsleep);
  }
  else if (idx == 3) {
    runner("resched", resched);
  }
  else {
    t__print("\n----------------------------\n");
    feedback("General", general);
    feedback("Sleep", sleep);
    feedback("Unsleep", unsleep);
    feedback("Resched", resched);
    t__print("\n");
  }
}
