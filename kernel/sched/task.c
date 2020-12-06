#include <syscalls.h>
#include <task.h>
#include <errno.h>
#include <arch.h>
#include <arch/context.h>
#include <sched.h>
#include <memory.h>

#define KERNEL_STACK_SIZE 8192


int task_schedulable(task_t* task)
{
  if (task->waiting_for_lock)
    return false;
  return (task->state == TASK_RUNNING);
}

static size_t get_task_id()
{
  static size_t tid_counter = 1;
  return atomic_add(&tid_counter, 1);
}

size_t create_ktask(void (*func)())
{
  task_t* task  = kmalloc(sizeof(task_t));
  task->state   = TASK_RUNNING;
  task->tid     = get_task_id();

  task->waiting_for_lock = false;

  task->kernel_stack = kmalloc(KERNEL_STACK_SIZE);
  task->kernel_stack_size = KERNEL_STACK_SIZE;

  void* stack_ptr = task->kernel_stack + task->kernel_stack_size;

  task->vspace  = VSPACE_KERNEL;
  task->context = ctx_create((void*)func, stack_ptr, stack_ptr, CTX_KERNEL);

  sched_insert(task);
  debug(SCHED, "task %zd: inserted\n", task->tid);
  return task->tid;
}


void task_iowait_if(size_t *mem, size_t value)
{
  int wait = false;

  preempt_disable();
  if (*mem == value)
  {
    current_task->waiting_for_lock = true;
    wait = true;
  }
  preempt_enable();

  if (wait)
    yield();
}

void task_iowake(task_t* task)
{
  task->waiting_for_lock = false;
}
