#include <util/types.h>
#include <sched/task.h>
#include <sched/proc.h>
#include <sched/sched.h>
#include <mm/memory.h>
#include <mm/vspace.h>
#include <arch/common.h>

static size_t pid_counter = 1;

void proc_start(const char *filename)
{
  /* create an ELF loader object to load
   * the binary from file. */
  void* entry_addr = 0;
  void* stack_ptr = 0;

  /* create a new process object and initialize
   * all the fields */
  proc_t* proc = kmalloc(sizeof(proc_t));
  proc->pid = atomic_add(&pid_counter, 1);

  list_init(&proc->task_list);
  mutex_init(&proc->task_list_lock);

  /* allocate new virtual address space
   * for the process. */
  proc->vspace = vspace_create();

  /* create a new stack for the main thread. */
  // TODO

  /* create the main thread and insert it into
   * the scheduler. */
  task_t* main_thread = create_user_task(proc->vspace, entry_addr, stack_ptr);
  list_add(&proc->task_list, main_thread);
  //sched_insert(main_thread);
}
