#pragma once

#include <sched/proc.h>

// Allow a stack of 32MB
#define STACK_SIZE    (32*1024*1024)

typedef struct
{
  /* the index of the stack is the index of
   * stack slots times two from the user break
   * downwards. (times two because there's a 32MB
   * guard slot between each 32MB stack). */
  size_t index;

  /* the thread that owns the stack */
  size_t tid;

  /* convenience variables that store the
   * start of the memory block as well as
   * the end (where the initial stack pointer
   * will be placed. */
  size_t start_page;
  size_t page_count;
  void* stack_ptr;

  /* specifies whether the stack is allocated
   * or can be used by a new thread. */
  int allocated;
} userstack_t;

userstack_t* create_stack(proc_t *process);
