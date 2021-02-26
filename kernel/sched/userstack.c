#include <sched/userstack.h>
#include <mm/memory.h>

userstack_t *create_stack(proc_t* proc, size_t tid)
{
  // TODO: assert that the process pointer lock is held

  userstack_t* stack = kmalloc(sizeof(userstack_t));
  stack->tid = tid;


  // TODO: find stack slot algorithm
  size_t min_free_index = 0;
  for (size_t index = 0; ; index++)
  {
    for (list_item_t* it = list_it_front(&proc->stack_list);
         it != LIST_IT_END;
         it = list_it_next(it))
    {
      userstack_t* stack = list_it_get(it);
      if (stack->index > min_free_index)
        min_free_index = stack->index + 1;
    }
  }

}
