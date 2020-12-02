#include <sched.h>
#include <types.h>
#include <task.h>
#include <list.h>
#include <mutex.h>
#include <vspace.h>
#include <arch/context.h>
#include <arch.h>

task_t* current_task;
vspace_t* current_vspace;
arch_context_t* saved_context;
static int sched_enabled = 0;
static int sched_blocked = 0;
static list_t* task_list_;


void sched_init(task_t *initial)
{
  debug(SCHED, "setting up scheduler task list\n");
  task_list_ = list_init();
}

void sched_start()
{
  debug(SCHED, "enabling scheduler\n");
  assert(list_size(task_list_) > 0, "no tasks to schedule!");
  sched_enabled = 1;

  yield();
}

void sched_insert(task_t* task)
{
  sched_blocked = true;
  list_add(task_list_, task);
  sched_blocked = false;
}

static task_t* get_next_task()
{
  task_t* next_task;
  do
  {
    list_rotate(task_list_);
  }
  while ((next_task = list_get(task_list_, 0))
         ->state != TASK_RUNNING);

  return next_task;
}

void schedule()
{
  if (!sched_enabled || sched_blocked)
    return;

  task_t* next_task = get_next_task();

  if (next_task != current_task)
  {
    current_task = next_task;

    // set the context
    saved_context = next_task->context;

    // set the interrupt stack
    ctx_set_kernel_stack(next_task->kernel_stack
      + next_task->kernel_stack_size);

    // switch address spaces if necessary
    if (current_vspace != next_task->vspace)
      vspace_apply(next_task->vspace);
  }
}

void yield()
{
  arch_yield();
}

void set_context(arch_context_t* context)
{
  if (current_task)
    current_task->context = context;
  saved_context = context;
}

arch_context_t* get_context()
{
  return saved_context;
}

void idle_task()
{
  char* videomem = (char*)0xb8000;
  char* seq = "|/-\\|/-\\";
  int x = 0, y = 0;
  while (1)
  {
    if (seq[x] == 0)
      x = 0;
    char c = seq[x];

    if (y++ > 2)
    {
      y = 0;
      x++;
    }

    *videomem = c;
    arch_idle();
  }
}
