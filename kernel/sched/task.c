#include <syscalls.h>
#include <task.h>
#include <errno.h>
#include <arch.h>
#include <arch/context.h>
#include <memory.h>

#define KERNEL_STACK_SIZE 4096
#define KTASK_STACK_SIZE  8192

arch_context_t* saved_context_;

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

static size_t get_task_id()
{
  static size_t tid_counter = 1;
  return atomic_add(&tid_counter, 1);
}

size_t create_ktask(void* func)
{
  task_t* task  = kmalloc(sizeof(task_t));
  task->state   = TASK_RUNNING;
  task->tid     = get_task_id();

  task->kernel_stack = kmalloc(KERNEL_STACK_SIZE);
  task->ktask_stack  = kmalloc(KTASK_STACK_SIZE);

  task->context = ctx_create(func, task->ktask_stack, CTX_KERNEL);

  // insert task
  return task->tid;
}

void set_context(arch_context_t* context)
{
  saved_context_ = context;
}

arch_context_t* get_context()
{
  return saved_context_;
}
