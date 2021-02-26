#include <sched/task.h>
#include <sched/proc.h>
#include <sched/sched.h>
#include <sched/tasklist.h>
#include <mm/memory.h>
#include <arch/context.h>
#include <arch/common.h>
#include <debug.h>

static size_t tid_counter = 1;

static void ktask_runtime(void (*func)())
{
  func();

  debug(TASK, "kernel task TID %zu completed. now terminating\n",
        current_task->tid);
  task_kill();
}

task_t* create_kernel_task(void (*func)())
{
  task_t* task = kmalloc(sizeof(task_t));
  task->kstack_base = ppn_to_virt(alloc_page());
  task->kstack_ptr = task->kstack_base + PAGE_SIZE;
  task->context = context_init(
    task->kstack_ptr,   // the kernel stack for this task
    ktask_runtime,      // the entry point (kernel task runtime)
    task->kstack_ptr,   // the stack pointer for this task
    false,              // not a user task
    (size_t)func, 0     // function to be called by the runtime
  );
  task->state = TASK_RUNNING;
  task->tid = atomic_add(&tid_counter, 1);
  task->vspace = VSPACE_KERNEL;
  tl_insert(task);
  debug(TASK, "created new kernel task with TID #%zu\n", task->tid);
  return task;
}

task_t* create_user_task(vspace_t* vspace, void* entry, void* stack_ptr)
{
  task_t* task = kmalloc(sizeof(task_t));
  task->kstack_base = ppn_to_virt(alloc_page());
  task->kstack_ptr = task->kstack_base + PAGE_SIZE;
  task->context = context_init(
    task->kstack_ptr,   // the kernel stack for this task
    entry,              // the entry point (ELF entry)
    stack_ptr,          // the stack pointer for this task
    true,               // is a user task
    0, 0                // no initial parameters
  );
  task->state = TASK_RUNNING;
  task->tid = atomic_add(&tid_counter, 1);
  task->vspace = vspace;
  tl_insert(task);
  debug(TASK, "created new user task with TID #%zu\n", task->tid);
  return task;
}

void task_kill()
{
  debug(TASK, "task #%zu terminated\n", current_task->tid);
  current_task->state = TASK_KILLED;
  yield();
}

int task_schedulable(task_t *task)
{
  /* this function is called by the schedule()
   * and therefore is running in an interrupt
   * context with further interrupts disabled.
   * don't use any locks and don't block */

  if (task->state == TASK_RUNNING)
    return true;
  return false;
}
