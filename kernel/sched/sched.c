#include <sched/sched.h>
#include <sched/tasklist.h>
#include <arch/context.h>
#include <sched/task.h>
#include <util/list.h>
#include <mm/vspace.h>

#include <debug.h>

static list_t sched_tasks;
static int sched_enabled = false;
task_t* current_task = NULL;

static void idle_task_func()
{
  for (;;)
  {
    idle();
  }
}

void sched_init()
{
  debug(SCHED, "setting up scheduler\n");

  /* initialize the list of schedulable tasks. */
  list_init(&sched_tasks);

  sched_enabled = false;

  /* setup the task list, which acts as a task garbage
   * collector. this must happen before any tasks are
   * created. */
  tl_setup();

  /* install the idle task. this task will halt the
   * cpu until an interrupt or exception fires over
   * and over again. */
  task_t* idle_task = create_kernel_task(idle_task_func);
  sched_insert(idle_task);
}

static task_t* get_next_task()
{
  /* this function determines the scheduling
   * policy. currently, this is round robin. */

  task_t* next;
  do
  {
    next = list_get(&sched_tasks, 0);
    assert(next, "no schedulable task");
    list_rotate(&sched_tasks);
  }
  while (!task_schedulable(next));

  return next;
}

context_t* schedule(context_t* ctx)
{
  /* the schedule() function determines the next
   * thread to be scheduled. this function is always
   * called with interrupts disabled. */
  if (!sched_enabled)
    return ctx;

  task_t* next_task = get_next_task();

  if (current_task != next_task)
  {
    if (current_task)
    {
      current_task->context = ctx;

      if (current_task->state == TASK_KILLED)
      {
        /* if the task was killed, remove it from the
         * scheduler to cleanup the scheduler task list */
        list_it_remove(&sched_tasks, list_find(&sched_tasks, current_task));
      }
    }

    /* update the context (registers and state)
     * that will be loaded when performing a
     * context switch. */
    ctx = next_task->context;

    /* if different, switch the virtual address
     * space to the one of the new task. */
    if (!current_task || current_task->vspace != next_task->vspace)
      vspace_apply(next_task->vspace);

    current_task = next_task;
  }

  return ctx;
}

void sched_enable()
{
  sched_enabled = true;
  yield();
}

void sched_insert(task_t *task)
{
  preempt_disable();
  list_add(&sched_tasks, task);
  preempt_enable();
}
