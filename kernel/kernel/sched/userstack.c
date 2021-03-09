#include <sched/userstack.h>
#include <mm/memory.h>
#include <errno.h>
#include <debug.h>

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
      (USER_BREAK >> PAGE_SHIFT) - 2 * (index + 1) * page_count;

  userstack_t* stack = kmalloc(sizeof(userstack_t));
  stack->start_page = start_page;
  stack->page_count = page_count;
  stack->index = index;
  stack->allocated = true;
  stack->pages_mapped = 0;
  stack->stack_ptr =
      (void*)((start_page + (STACK_SIZE >> PAGE_SHIFT)) << PAGE_SHIFT);
  list_add(&proc->stack_list, stack);

  mutex_unlock(&proc->stack_list_lock);
  return stack;
}

int stack_load(proc_t *proc, size_t virt_page)
{
  int ret = -ENOENT;
  mutex_lock(&proc->stack_list_lock);
  for (list_item_t* it = list_it_front(&proc->stack_list);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    userstack_t* stack = list_it_get(it);
    if (stack->allocated &&
        virt_page >= stack->start_page &&
        virt_page < stack->start_page + stack->page_count)
    {
      size_t ppn = alloc_page();
      vspace_map(proc->vspace, virt_page, ppn, PG_NOEXEC | PG_USER | PG_WRITE);
      stack->pages_mapped++;
      ret = SUCCESS;
      break;
    }
  }
  mutex_unlock(&proc->stack_list_lock);
  return ret;
}

void delete_stack(proc_t *process, size_t index)
{
  mutex_lock(&process->stack_list_lock);
  userstack_t* stack = list_get(&process->stack_list, index);
  assert(stack && stack->allocated, "stack index invalid or not allocated");
  stack->allocated = 0;

  /* unmap all the stack pages */
  debug(TASK, "unwinding stack of PID %zu and index %zu\n",
        process->pid, index);
  size_t end_page = stack->start_page + stack->page_count - 1;
  while (stack->pages_mapped)
  {
    if (vspace_unmap(process->vspace, end_page--))
      stack->pages_mapped -= 1;
  }

  list_item_t* it;
  for (;;)
  {
    if ((it = list_it_back(&process->stack_list)) == LIST_IT_END)
      break;

    userstack_t* last_stack = list_it_get(it);
    if (last_stack->allocated)
      break;

    list_it_remove(&process->stack_list, it);
  }
  mutex_unlock(&process->stack_list_lock);
}
