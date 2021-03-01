#include <sched/userstack.h>
#include <mm/memory.h>

userstack_t *create_stack(proc_t* proc)
{
  mutex_lock(&proc->stack_list_lock);
  for (list_item_t* it = list_it_front(&proc->stack_list);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    userstack_t* stack = list_it_get(it);
    if (!stack->allocated)
    {
      stack->allocated = true;
      mutex_unlock(&proc->stack_list_lock);
      return stack;
    }
  }

  const size_t index = list_size(&proc->stack_list);
  const size_t page_count = STACK_SIZE  / PAGE_SIZE;
  const size_t start_page =
      (USER_BREAK >> PAGE_SHIFT) - index * page_count;

  userstack_t* stack = kmalloc(sizeof(userstack_t));
  stack->start_page = start_page;
  stack->page_count = page_count;
  stack->index = index;
  stack->allocated = true;
  stack->stack_ptr = (void*)(USER_BREAK - (index * STACK_SIZE));

  mutex_unlock(&proc->stack_list_lock);
  return stack;
}
