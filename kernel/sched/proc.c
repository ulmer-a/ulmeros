#include <util/types.h>
#include <sched/task.h>
#include <sched/proc.h>
#include <sched/sched.h>
#include <sched/loader.h>
#include <mm/memory.h>
#include <mm/vspace.h>
#include <fs/vfs.h>
#include <arch/common.h>
#include <bits.h>
#include <debug.h>

static size_t pid_counter = 1;

void proc_start(const char *filename)
{
  /* create an ELF loader object to load
   * the binary from file. */
  fd_t* fd;
  if (vfs_open(filename, O_RDONLY, 0, &fd) < 0)
  {
    assert(false, "cannot open init binary");
    return;
  }

  loader_t* ldr = loader_create(fd);
  if (ldr == NULL)
  {
    assert(false, "elf-loader: invalid executable format");
    return;
  }

  /* create a new process object and initialize
   * all the fields */
  proc_t* proc = kmalloc(sizeof(proc_t));
  proc->pid = atomic_add(&pid_counter, 1);
  proc->loader = ldr;

  list_init(&proc->task_list);
  mutex_init(&proc->task_list_lock);

  /* allocate new virtual address space
   * for the process. */
  proc->vspace = vspace_create();

  /* create a new stack for the main thread. */
  // TODO
  void* stack_ptr = 0;

  /* create the main thread and insert it into
   * the scheduler. */
  task_t* main_thread = create_user_task(proc->vspace, ldr->entry_addr, stack_ptr);
  list_add(&proc->task_list, main_thread);
  //sched_insert(main_thread);
}
