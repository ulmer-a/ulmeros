#include <util/types.h>
#include <debug.h>
#include <sched/task.h>
#include <sched/userstack.h>
#include <mm/memory.h>

static const char* bool_str(int bool)
{
  return bool ? "yes" : "no";
}

int page_fault(size_t address, int present, int write, int user, int exec)
{
  debug(PAGEFAULT, "addr=%p, present=%s, write=%s, user=%s, exec=%s\n",
        address, bool_str(present), bool_str(write),
        bool_str(user), bool_str(exec));
  const size_t virt_page = address >> PAGE_SHIFT;

  if (!current_task || !current_task->process)
    return false;
  proc_t* process = current_task->process;

  /* check whether the fault is caused by reading or
   * writing to a userspace stack page. */
  if (stack_load(process, virt_page) >= 0)
    return true;

  if (heap_load(process, address) >= 0)
    return true;

  /* check whether the fault is caused by a missing code
   * or data page. it will be reloaded from the binary. */
  if (loader_load(process->loader, virt_page, current_task->vspace) >= 0)
    return true;

  return false;
}
