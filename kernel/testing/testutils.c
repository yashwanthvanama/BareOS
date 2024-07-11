#include "complete.h"
#include <barelib.h>
#ifdef MILESTONE_3
#include <thread.h>
#endif
#ifdef MILESTONE_4
#include <queue.h>
#endif

#define assert(test, ls, err) if (!(test)) ls = err

void t__reset(uint32);

char __real_uart_putc(char);
void t__print(const char* str) {
  while (*str != '\0') __real_uart_putc(*str++);
}

char* t__strcpy(char* str, const char* value) {
  while ((*(str++) = *(value++)) != '\0');
  return str - 1;
}

char* t__hexcpy(char* str, unsigned int v) {
  int len = 0;
  int vals[20];
  for (int i=0; i<20; i++) vals[i] = 0;
  while (v) {
    vals[len++] = v % 16;
    v = v / 16;
  }
  int l = len - 1;
  for (; l >=0; l--) {
    *(str++) = vals[l] <= 9 ? vals[l] + '0' : 'a' + (vals[l] - 10);
  }
  return str;
}

char* t__intcpy(char* str, int v) {
  int len = 0;
  int vals[20];
  if (v < 0)
    *(str++) = '-';
  for (int i=0; i<20; i++) vals[i] = 0;
  while (v) {
    vals[len++] = v % 10;
    v = v / 10;
  }
  int l = len - 1;
  for (; l >=0; l--)
    *(str++) = vals[l] + '0';
  return str;
}

#ifdef MILESTONE_3
static thread_t t__thread_table[NTHREADS];
static uint32 t__current_thread;
#endif
#ifdef MILESTONE_4
static queue_t t__thread_queue[NTHREADS+2];
#endif

void t__mem_snapshot(void) {
#ifdef MILESTONE_3
  t__current_thread = current_thread;
  for (int i=0; i<NTHREADS; i++) {
    t__thread_table[i].state = thread_table[i].state;
    t__thread_table[i].parent = thread_table[i].parent;
    t__thread_table[i].priority = thread_table[i].priority;
    t__thread_table[i].retval = thread_table[i].retval;
    t__thread_table[i].stackptr = thread_table[i].stackptr;
  }
#endif
#ifdef MILESTONE_4
  uint32 end = NTHREADS+1;
#ifdef MILESTONE_5
  end += 1;
#endif
  for (int i=0; i<end; i++) {
    t__thread_queue[i].qnext = thread_queue[i].qnext;
    t__thread_queue[i].qprev = thread_queue[i].qnext;
  }
#endif
}

void t__mem_reset(void) {
#ifdef MILESTONE_3
  current_thread = 0;
  for (int i=0; i<NTHREADS; i++) {
    thread_table[i].state = TH_FREE;
    thread_table[i].parent = 0;
    thread_table[i].priority = 0;
    thread_table[i].retval = 0;
    thread_table[i].stackptr = 0;
  }
#endif
#ifdef MILESTONE_4
  uint32 end = NTHREADS+1;
#ifdef MILESTONE_5
  end += 1;
#endif
  for (int i=0; i<end; i++)
    thread_queue[i].qnext = thread_queue[i].qprev = i;
#endif
}

void t__mem_restore(void) {
#ifdef MILESTONE_3
  current_thread = t__current_thread;
  for (int i=0; i<NTHREADS; i++) {
    thread_table[i].state = t__thread_table[i].state;
    thread_table[i].parent = t__thread_table[i].parent;
    thread_table[i].priority = t__thread_table[i].priority;
    thread_table[i].retval = t__thread_table[i].retval;
    thread_table[i].stackptr = t__thread_table[i].stackptr;
  }
#endif
#ifdef MILESTONE_4
  uint32 end = NTHREADS+1;
#ifdef MILESTONE_5
  end += 1;
#endif
  for (int i=0; i<end; i++) {
    thread_queue[i].qnext = t__thread_queue[i].qnext;
    thread_queue[i].qprev = t__thread_queue[i].qnext;
  }
#endif
}

void t__mark_timeout(byte idx, char** arr, uint32 len) {
  if (idx >= 0 && idx < len) {
    assert(0, arr[idx], "TIMEOUT - The test timed out before completion and never returned");
  }
  else {
    for (int i=0; i<len; i++)
      assert(0, arr[i], "TIMEOUT - The test timed out before completion and never returned");
  }
}

static uint32 run_idx = 0;
void t__runner(const char* label, uint32 count, void (*runner)(void)) {
  char num[10];
  for (int i=0; i<10; i++) num[i] = '\0';
  t__intcpy(num, (int)count);
  t__print("Running [");
  t__print(num);
  t__print("] ");
  t__print(label);
  t__print(" tests...");
  runner();
  t__print(" DONE\n");
  t__reset(++run_idx);
}

void t__printer(const char* header, char* out[], const char* in[], uint32 count) {
  t__print(header);
  t__print("\n");
  for (int i=0; i<count; i++) {
    t__print(in[i]);
    t__print(out[i]);
    t__print("\n");
  }
}
