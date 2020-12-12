#include <sched/userstack.h>
#include <memory.h>

#define DEFAULT_SIZE  2048  // 2048 pages = 8MB
#define DEFAULT_GUARD 16    // guard pages

ustack_t* userstack_create(proc_t* proc)
{
  mutex_lock(&proc->stacks_lock);

  /* first, try if we can find a stack slot that
   * suits our needs. */
  ustack_t* last_stack;
  size_t stack_count = list_size(proc->stacks);
  for (size_t i = 0; i < stack_count; i++)
  {
    last_stack = list_get(proc->stacks, i);
    if (!last_stack->present)
    {
      last_stack->present = 1;
      mutex_unlock(&proc->stacks_lock);
      return last_stack;
    }
  }

  /* if not, we calculate the next free location
   * to use for stacks. */
  size_t next_stack = last_stack->start_page
      - last_stack->guard_count;

  /* then allocate a new stack */
  ustack_t* stack = kmalloc(sizeof(ustack_t));
  stack->present = 1;
  stack->page_count = DEFAULT_SIZE;
  stack->guard_count = DEFAULT_GUARD;
  stack->start_page = next_stack - stack->page_count;

  mutex_unlock(&proc->stacks_lock);
  return stack;
}

void userstack_delete(ustack_t* stack)
{
  (void)stack;
  assert(false, "stack deletion is not implemented");
}
