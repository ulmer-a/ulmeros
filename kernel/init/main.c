#include <debug.h>
#include <cmdline.h>
#include <sched/sched.h>
#include <sched/task.h>
#include <sched/proc.h>
#include <sched/tasklist.h>
#include <arch/common.h>

static void idle_task_func()
{
  for (;;)
  {
    debug(INIT, "welcome from the idle thread\n");
    idle();
  }
}

static void init_task_func()
{
  debug(INIT, "Welcome from the init kernel task\n");

  // 1. Delete old stack

  /* obtain the name of the init program to
   * run as PID 1, which is passed on the kernel
   * command line. */
  const char* init_program = cmdline_get("init");
  assert(init_program, "cmdline: no 'init' binary");
  proc_start(init_program);
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

  tl_setup();

  task_t* idle_task = create_kernel_task(idle_task_func);
  sched_insert(idle_task);

  task_t* init_task = create_kernel_task(init_task_func);
  sched_insert(init_task);

  /* enable the scheduler and yield(). this will
   * not return. */
  sched_enable();
}
