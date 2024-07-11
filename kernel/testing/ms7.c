#include <barelib.h>
#include <thread.h>
#include <malloc.h>
#include "tests.h"

static alloc_t* freelist;
extern uint32* mem_start;
extern uint32* mem_end;

static const char* general_prompt[] = {
				       "  Program Compiles:                      ",
				       "  Heap initialized during boot:          ",
};
static const char* malloc_prompt[] = {
				       "  First small allocation correct:        ",
				       "  Second small allocation correct:       ",
				       "  Too big returns NULL:                  ",
				       "  Completely fill heap:                  ",
				       "  New block between allocations (small): ",
				       "  New block between allocations (fit):   ",
};
static const char* free_prompt[] = {
				       "  Free last block:                       ",
				       "  Free middle block:                     ",
				       "  Coalesce down:                         ",
				       "  Coalesce up:                           ",
				       "  Free first:                            ",
};

static char* general_t[test_count(general_prompt)];
static char* malloc_t[test_count(malloc_prompt)];
static char* free_t[test_count(free_prompt)];

static void mem_reset(uint32 sz) {
  heap_init();
  freelist = (alloc_t*)mem_start;
  freelist->size = sz;
}

static void general_tests(void) {
  freelist = (alloc_t*)mem_start;
  assert(freelist->state == M_FREE, general_t[1],                                           "FAIL - 'freelist' state was not initialized");
  assert(freelist->size == get_stack(NTHREADS) - mem_start - sizeof(alloc_t), general_t[1], "FAIL - 'freelist' size does not match heap size");
  assert(freelist->next == NULL, general_t[1],                                              "FAIL - 'freelist' next is non-null value");
}

static void malloc_tests(void) {
  mem_reset(120);
  char* old_list = (char*)freelist;
  alloc_t* header = (alloc_t*)old_list;
  char* ptr = (char*)t__with_timeout(10, malloc, 20);
  freelist = (alloc_t*)(ptr + 20);

  maybe_timeout(0, malloc) {
    assert(ptr == (old_list + sizeof(alloc_t)),    malloc_t[0], "FAIL - Returned pointer is not expected first address");
    assert(ptr != old_list,                        malloc_t[0], "FAIL - Returned pointer is not offset by the alloc_t structure");
    assert(header->size < 21,                      malloc_t[0], "FAIL - Allocated block size is too big");
    assert(header->size > 19,                      malloc_t[0], "FAIL - Allocated block size is too small");
    assert(header->state == M_USED,                malloc_t[0], "FAIL - Allocated block state was not M_USED");
    assert((char*)freelist == ptr + 20,            malloc_t[0], "FAIL - 'freelist' was not moved to the correct address");
    assert((char*)freelist != old_list,            malloc_t[0], "FAIL - 'freelist' was not moved after allocation");
    assert(freelist->state == M_FREE,              malloc_t[0], "FAIL - 'freelist' state was not M_FREE");
    assert(freelist->next == NULL,                 malloc_t[0], "FAIL - 'freelist' next was not NULL");
    assert(freelist->size < 101 - sizeof(alloc_t),  malloc_t[0], "FAIL - 'freelist' size is too big after allocation");
    assert(freelist->size > 99 - sizeof(alloc_t),  malloc_t[0], "FAIL - 'freelist' size is too small after allocation");
  }
  
  ptr = (char*)t__with_timeout(10, malloc, 40);
  maybe_timeout(1, malloc) {
    freelist = (alloc_t*)(ptr + 40);
    header = (alloc_t*)(ptr - sizeof(alloc_t));
    assert(ptr == (old_list + (sizeof(alloc_t) * 2) + 20), malloc_t[1], "FAIL - Returned pointer is not expected offset");
    assert(header->size < 41,                              malloc_t[1], "FAIL - Allocation block size is too big");
    assert(header->size > 39,                              malloc_t[1], "FAIL - Allocation block size is too small");
    assert(header->state == M_USED,                        malloc_t[1], "FAIL - Allocation block state was not M_USED");
    assert(freelist->state == M_FREE,                      malloc_t[1], "FAIL - 'freelist' state was not M_FREE");
    assert(freelist->next == NULL,                         malloc_t[1], "FAIL - 'freelist' next was not NULL");
    assert(freelist->size < 61 - (sizeof(alloc_t) * 2),    malloc_t[1], "FAIL - 'freelist' size is too big after allocation");
    assert(freelist->size > 59 - (sizeof(alloc_t) * 2),    malloc_t[1], "FAIL - 'freelist' size is too small after allocation");
  }
  
  ptr = (char*)t__with_timeout(10, malloc, 50);
  maybe_timeout(2, malloc) {
    assert(ptr == NULL, malloc_t[2], "FAIL - Did not return NULL when allocation is too big");
  }
  
  mem_reset(100);
  header = (alloc_t*)old_list;
  ptr = (char*)t__with_timeout(10, malloc, 100);
  maybe_timeout(3, malloc) {
    assert(header->size == 100,     malloc_t[3], "FAIL - Allocated block size was not correct");
    assert(header->state == M_USED, malloc_t[3], "FAIL - Allocation state was not set to M_USED");
  }
  
  mem_reset(400);
  ptr = ((char*)header) + sizeof(alloc_t);
  alloc_t *s1 = (alloc_t*)(ptr + 40);
  alloc_t *s2 = (alloc_t*)(ptr + 140);

  s1->size = 20;
  s1->state = M_FREE;
  s1->next = s2;
  s2->size = 60;
  s2->state = M_FREE;
  s2->next = NULL;
  header->size = 5;
  header->state = M_FREE;
  header->next = s1;

  ptr = (char*)t__with_timeout(10, malloc, 25);
  maybe_timeout(4, malloc) {
    assert(s2->size == 25, malloc_t[4], "FAIL - Allocation size does not match request");
    assert(s2->state == M_USED,  malloc_t[4], "FAIL - Allocation not placed correctly");
    assert(s1->state != M_USED,  malloc_t[4], "FAIL - Allocation placed in a too small block");
  }
  
  ptr = (char*)s1->next;
  t__with_timeout(10, malloc, 20);
  maybe_timeout(5, malloc) {
    assert(s1->state == M_USED, malloc_t[5], "FAIL - Allocation not placed correctly");
  }
}

static void free_tests(void) {
  mem_reset(400);
  freelist = (alloc_t*)mem_start;
  uint32 sz = 40 - sizeof(alloc_t);
  alloc_t* s1 = (alloc_t*)((char*)freelist + 30 + (0 * 40));
  alloc_t* s2 = (alloc_t*)((char*)freelist + 30 + (1 * 40));
  alloc_t* s3 = (alloc_t*)((char*)freelist + 30 + (2 * 40));
  alloc_t* s4 = (alloc_t*)((char*)freelist + 30 + (3 * 40));
  alloc_t* s5 = (alloc_t*)((char*)freelist + 30 + (4 * 40));

  freelist->size = 30 - sizeof(alloc_t);
  s1->size = sz;
  s1->state = M_USED;
  s1->next = NULL;
  s2->size = sz;
  s2->state = M_USED;
  s2->next = NULL;
  s3->size = sz;
  s3->state = M_USED;
  s3->next = NULL;
  s4->size = sz;
  s4->state = M_USED;
  s4->next = NULL;
  s5->size = sz;
  s5->state = M_USED;
  s5->next = NULL;

  t__with_timeout(10, free, (char*)s5 + sizeof(alloc_t));
  maybe_timeout(0, free) {
    assert(s5->size == sz,       free_t[0], "FAIL - Freed block was not the correct size");
    assert(s5->state == M_FREE,  free_t[0], "FAIL - Freed block was not marked as free");
    assert(s5->next == NULL,     free_t[0], "FAIL - Last free block's next value was not NULL");
    assert(freelist->next == s5, free_t[0], "FAIL - Newly freed block not linked to free list");
  }
  
  t__with_timeout(10, free, (char*)s3 + sizeof(alloc_t));
  maybe_timeout(1, free) {
    assert(s3->size == sz,       free_t[1], "FAIL - Freed block's size was changed");
    assert(s3->state == M_FREE,  free_t[1], "FAIL - Freed block was not marked as free");
    assert(s3->next == s5,       free_t[1], "FAIL - Freed block not connected to next free block");
    assert(freelist->next == s3, free_t[1], "FAIL - Newly freed block not linked to free list");
  }
  
  t__with_timeout(10, free, (char*)s4 + sizeof(alloc_t));
  maybe_timeout(2, free) {
    assert(s3->size == (sz * 3) + (sizeof(alloc_t) * 2), free_t[2], "FAIL - Did not coalesce free blocks");
    assert(s3->size != (sz * 2) + sizeof(alloc_t),       free_t[2], "FAIL - Freed block did not coalesce down");
  }
  maybe_timeout(3, free) {
    assert(s3->size == (sz * 3) + (sizeof(alloc_t) * 2), free_t[3], "FAIL - Did not coalesce free blocks");
    assert(s3->size != sz,                               free_t[3], "FAIL - Freed block did not coalesce up");
  }

  t__with_timeout(10, free, (char*)s1 + sizeof(alloc_t));
  maybe_timeout(4, free) {
    assert(freelist->next == s3, free_t[4], "FAIL - Freed block was not linked to the free list");
    assert(freelist->size == 46, free_t[4], "FAIL - Freed block was not coalesced into free list");
  }
}

void t__ms7(uint32 idx) {
  
  if (idx == 0) {
    t__print("\n");
    runner("general", general);
  }
  else if (idx == 1) {
    runner("malloc", malloc);
  }
  else if (idx == 2) {
    runner("free", free);
  }
  else {
    t__print("\n----------------------------\n");
    feedback("General", general);
    feedback("Malloc", malloc);
    feedback("Free", free);
    t__print("\n");
  }
}
