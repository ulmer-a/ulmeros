#include <syscalls.h>
#include <task.h>
#include <errno.h>
#include <arch.h>
#include <arch/context.h>
#include <sched.h>
#include <memory.h>

#define KERNEL_STACK_SIZE 4096
#define KTASK_STACK_SIZE  8192

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
  task->kernel_stack_size = KERNEL_STACK_SIZE;

  task->ktask_stack  = kmalloc(KTASK_STACK_SIZE);

  task->vspace  = VSPACE_KERNEL;
  task->context = ctx_create(func, task->kernel_stack
    + task->kernel_stack_size, task->ktask_stack, CTX_KERNEL);

  sched_insert(task);
  debug(SCHED, "task %zd: inserted\n", task->tid);
  return task->tid;
}
