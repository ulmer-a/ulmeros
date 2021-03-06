#include <util/types.h>
#include <sched/task.h>
#include <sched/proc.h>
#include <sched/sched.h>
#include <sched/loader.h>
#include <sched/userstack.h>
#include <mm/memory.h>
#include <mm/vspace.h>
#include <fs/vfs.h>
#include <arch/common.h>
#include <debug.h>
#include <syscalls.h>
#include <errno.h>

static size_t pid_counter = 1;

int proc_start(const char *filename)
{
  /* create an ELF loader object to load
   * the binary from file. */
  fd_t* fd;
  int error;
  if ((error = vfs_open(filename, O_RDONLY, 0, &fd)) < 0)
  {
    return error;
  }

  loader_t* ldr = loader_create(fd);
  if (ldr == NULL)
  {
    return -ENOEXEC;
  }

  /* create a new process object and initialize
   * all the fields */
  proc_t* proc = kmalloc(sizeof(proc_t));
  proc->pid = atomic_add(&pid_counter, 1);
  proc->loader = ldr;
  proc->state = PROC_RUNNING;

  list_init(&proc->task_list);
  mutex_init(&proc->task_list_lock);

  /* allocate new virtual address space
   * for the process. */
  proc->vspace = vspace_create();

  /* create a new stack for the main thread. */
  list_init(&proc->stack_list);
  mutex_init(&proc->stack_list_lock);
  userstack_t* stack = create_stack(proc);

  /* create the main thread and insert it into
   * the scheduler. */
  task_t* main_thread = create_user_task(proc->vspace, ldr->entry_addr, stack);
  main_thread->process = proc;
  list_add(&proc->task_list, main_thread);

  sched_insert(main_thread);
  return SUCCESS;
}

fd_t *proc_get_fd(proc_t *process, int fd)
{
  assert(false, "fixme: implement");
  return NULL;
}

int proc_new_fd(proc_t *process, fd_t *fd)
{
  assert(false, "fixme: implement");
  return -1;
}

void sys_exit(int status)
{
  kpanic(current_task->process, "user task has no associated process");
  debug(PROCESS, "exit(%d) called by PID %zu\n", status, current_task->process);

  /* by setting the current process state to killed,
   * all other threads will be killed during the next
   * few time slices. */
  current_task->process->state = PROC_KILLED;
  task_kill();
}
