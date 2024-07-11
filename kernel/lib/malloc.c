#include <barelib.h>
#include <malloc.h>
#include <thread.h>
#include <bareio.h>

extern uint32* mem_start;
extern uint32* mem_end;
static alloc_t* freelist;

/*  Sets the 'freelist' to 'mem_start' and creates  *
 *  a free allocation at that location for the      *
 *  entire heap.                                    */
//--------- This function is complete --------------//
void heap_init(void) {
  freelist = (alloc_t*)mem_start;
  freelist->size = get_stack(NTHREADS) - mem_start - sizeof(alloc_t);
  freelist->state = M_FREE;
  freelist->next = NULL;
}

/*  Locates a free block large enough to contain a new    *
 *  allocation of size 'size'.  Once located, remove the  *
 *  block from the freelist and ensure the freelist       *
 *  contains the remaining free space on the heap.        *
 *  Returns a pointer to the newly created allocation     */
void* malloc(uint64 size) {
  alloc_t* current_allocation = freelist;
  while(1) {
    if (current_allocation->size >= size && current_allocation->state == M_FREE)
      break;
    else if(current_allocation->next == NULL)
      return 0;
    else
    {
      current_allocation = current_allocation->next;
    }
  }
  uint64 remaining_size = current_allocation->size - size;
  if(current_allocation->size == size || current_allocation->next->size < sizeof(alloc_t))
  {
    current_allocation->state = M_USED;
    return (void*)(int64)current_allocation + sizeof(alloc_t);
  }
  else {
    current_allocation->size = size;
    current_allocation->state = M_USED;
    alloc_t* new_allocation = (alloc_t*)((uint64)current_allocation + sizeof(alloc_t) + size);
    new_allocation->size = remaining_size - sizeof(alloc_t);
    new_allocation->next = current_allocation->next;
    new_allocation->state = M_FREE;
    current_allocation->next = new_allocation;
    return (void*)(int64)current_allocation + sizeof(alloc_t);
  }
}

/*  Free the allocation at location 'addr'.  If the newly *
 *  freed allocation is adjacent to another free          *
 *  allocation, coallesce the adjacent free blocks into   *
 *  one larger free block.                                */
void free(void* addr) {
  if(addr == NULL)
    return;
  alloc_t* current_allocation = (alloc_t*)((uint64)addr - sizeof(alloc_t));
  alloc_t* prev_allocation = freelist;
  alloc_t* next_allocation = freelist->next;
  while(next_allocation->next != NULL)
  {
    if(addr > (void*)prev_allocation->next)
    {
      prev_allocation = next_allocation;
      next_allocation = next_allocation->next;
    }
    else
      break;
  }
  current_allocation->next = next_allocation;
  prev_allocation->next = current_allocation;
  current_allocation->state = M_FREE;
  if ((uint64)current_allocation + current_allocation->size + sizeof(alloc_t) == (uint64)next_allocation && next_allocation->state == M_FREE) {
      current_allocation->size += next_allocation->size + sizeof(alloc_t);
      current_allocation->next = next_allocation->next;
  }
  if((uint64)prev_allocation + prev_allocation->size + sizeof(alloc_t) == (uint64)current_allocation && prev_allocation->state == M_FREE) {
    prev_allocation->next = current_allocation->next;
    prev_allocation->size += current_allocation->size + sizeof(alloc_t);
  }
}
