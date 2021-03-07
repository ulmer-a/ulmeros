#include <syscalls.h>
#include <util/types.h>
#include <sched/task.h>
#include <sched/proc.h>
#include <mm/vspace.h>
#include <mm/memory.h>
#include <debug.h>
#include <errno.h>

void* sys_sbrk(ssize_t increment)
{
  /* some sanity checks */
  proc_t* process = current_task->process;
  if (process->heap_brk == (size_t)-1)
    return (void*)-1;
  assert(process->loader, "sbrk(): process has invalid loader_t");

  /* get some values */
  const size_t min_heap_break = process->loader->min_heap_break;
  void* old_break = (void*)process->heap_brk;

  mutex_lock(&process->heap_lock);
  if (process->heap_brk + increment < min_heap_break)
    process->heap_brk = min_heap_break;
  else
    process->heap_brk = process->heap_brk + increment;

  debug(SYSCALL, "sys_sbrk(): moving heap break %p -> %p (%zd)\n",
        old_break, process->heap_brk, increment);

  /* if the sbrk() call is used to deallocate memory,
   * the corresponding pages are unmapped and further
   * references to them are considered invalid. */
  if (increment < 0)
  {
    size_t first_unmap = process->heap_brk >> PAGE_SHIFT;
    if (process->heap_brk % PAGE_SIZE != 0)
      first_unmap += 1;

    size_t last_unmap = (size_t)old_break >> PAGE_SHIFT;
    if ((size_t)old_break % PAGE_SIZE == 0)
      last_unmap -= 1;

    for (size_t page = first_unmap; page <= last_unmap; page++)
      vspace_unmap(process->vspace, page);
  }

  mutex_unlock(&process->heap_lock);
  return old_break;
}

int heap_load(proc_t* proc, size_t addr)
{
  int ret = -ENOENT;
  mutex_lock(&proc->heap_lock);
  if (addr >= proc->loader->min_heap_break && addr < proc->heap_brk)
  {
    size_t ppn = alloc_page();
    debug(LOADER, "PID %zu: heap: mapping page %zu @ %p\n",
          proc->pid, ppn, addr & ~0xfff);
    vspace_map(proc->vspace, addr >> PAGE_SHIFT, ppn, PG_WRITE|PG_USER);
    ret = SUCCESS;
  }
  mutex_unlock(&proc->heap_lock);
  return ret;
}
