#include <debug.h>
#include <cmdline.h>
#include <sched/sched.h>
#include <sched/task.h>
#include <arch/common.h>

static void idle_task_func()
{
  for (;;)
  {
    debug(INIT, "welcome from the idle thread\n");
    idle();
  }
}

void kmain(const char *cmdline)
{
  /*
   * this is the generic kernel initialization
   * routine. at this point, architecture specific
   * code has already initialized the processor,
   * interrupts and virtual memory. kmain()'s main
   * task is to parse command line arguments,
   * initialize the scheduler and start the first
   * kernel thread, which will then perform further
   * initialization and load device drivers.
   */

  debug(INIT, "reached kmain()\n");
  cmdline_parse(cmdline);

  /* initialize the scheduler's data structures. */
  sched_init();

  task_t* idle_task = create_kernel_task(idle_task_func);
  sched_insert(idle_task);

  /* enable the scheduler and yield(). this will
   * not return. */
  sched_enable();
}
