#include <errno.h>
#include <types.h>
#include <proc.h>
#include <memory.h>
#include <arch.h>
#include <task.h>
#include <sched/userstack.h>
#include <kstring.h>

static size_t proc_get_pid()
{
  static size_t pid_counter = 1;
  return atomic_add(&pid_counter, 1);
}

static void proc_add_mainthread(proc_t* proc, void* entry)
{
  ustack_t* stack = userstack_create(proc);
  void* stack_ptr = (void*)((stack->start_page +
    stack->page_count) << PAGE_SHIFT);

  create_utask(entry, stack_ptr, proc->addr_space);
}

size_t proc_create_initial(const char* filename)
{
  proc_t* process = kmalloc(sizeof(proc_t));

  int error;
  loader_t* loader;
  loader = loader_create(process, filename, &error);
  if (loader == NULL)
  {
    kfree(process);
    debug(LOADER, "error: %s: %s\n", filename, strerror(error));
    return 0;
  }

  process->pid = proc_get_pid();

  /* allocate lists */
  process->threads = list_init();
  process->stacks = list_init();

  /* initialize synchronization */
  mutex_init(&process->threads_lock);
  mutex_init(&process->stacks_lock);

  /* initialize the virtual address space */
  process->addr_space = vspace_init();
  process->loader = loader;

  /* create a main thread and insert it into
   * the scheduler. main threads are just like
   * every other thread, but they are created
   * in a different way. */
  size_t pid = process->pid;
  proc_add_mainthread(process, loader_get_entry(loader));
  return pid;
}

void sys_exit()
{

}

ssize_t sys_fork()
{
  return -ENOSYS;
}

int sys_exec(char *path, char** argv)
{
  return -ENOSYS;
}

int sys_wait(size_t pid, int* exit_code)
{
  return -ENOSYS;
}

size_t sys_getpid()
{
  return (size_t)-ENOSYS;
}
