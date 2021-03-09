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

static void proc_base_init(proc_t* proc)
{
  proc->state = PROC_RUNNING;
  proc->pid = atomic_add(&pid_counter, 1);

  list_init(&proc->task_list);
  list_init(&proc->fd_list);
  list_init(&proc->stack_list);

  mutex_init(&proc->heap_lock);
  mutex_init(&proc->task_list_lock);
  mutex_init(&proc->fd_list_lock);
  mutex_init(&proc->stack_list_lock);
}

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
  proc_base_init(proc);
  proc->fd_counter = 0;

  /* allocate new virtual address space
   * for the process. */
  proc->vspace = vspace_create();

  /* initialize the loader and get the heap
   * start address */
  proc->loader = ldr;
  proc->heap_brk = ldr->min_heap_break;

  /* create a new stack for the main thread. */
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
  if (fd < 0)
    return NULL;

  /* lookup the corresponding file descriptor
   * object in the process' list of fd's. */
  fd_t* ret = NULL;
  mutex_lock(&process->fd_list_lock);
  for (list_item_t* it = list_it_front(&process->fd_list);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    user_fd_t* user_fd = list_it_get(it);
    if (user_fd->user_fd == (size_t)fd)
    {
      ret = user_fd->fd_ptr;
      break;
    }
  }
  mutex_unlock(&process->fd_list_lock);
  return ret;
}

int proc_new_fd(proc_t *process, fd_t *fd)
{
  /* allocate a new file descriptor number by
   * atomically incrementing the process' counter */
  const size_t new_fd = atomic_add(&process->fd_counter, 1);

  /* create a new file descriptor list entry */
  user_fd_t* userfd = kmalloc(sizeof(user_fd_t));
  userfd->fd_ptr = fd;
  userfd->user_fd = new_fd;

  /* add the file descriptor to the process' list
   * of open files. */
  mutex_lock(&process->fd_list_lock);
  list_add(&process->fd_list, userfd);
  mutex_unlock(&process->fd_list_lock);

  return new_fd;
}

void sys_exit(int status)
{
  kpanic(current_task->process, "user task has no associated process");
  debug(PROCESS, "exit(%d) called by PID %zu\n",
        status, current_task->process->pid);

  /* by setting the current process state to killed,
   * all other threads will be killed during the next
   * few time slices. */
  current_task->process->state = PROC_KILLED;
  task_kill();
}
