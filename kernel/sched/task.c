#include <sched/task.h>
#include <mm/memory.h>
#include <arch/context.h>
#include <arch/common.h>

static size_t tid_counter = 1;

static void ktask_runtime(void (*func)())
{
  func();

  // kill the current task
}

static void mk_kernel_context(void* func)
{

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
  return task;
}

int task_schedulable(task_t *task)
{
  if (task->state == TASK_RUNNING)
    return true;
  return false;
}
