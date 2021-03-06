#include <debug.h>
#include <cmdline.h>
#include <sched/sched.h>
#include <sched/task.h>
#include <sched/proc.h>
#include <sched/interrupt.h>
#include <arch/common.h>
#include <arch/platform.h>
#include <fs/vfs.h>
#include <fs/ramdisk.h>
#include <fs/blockdev.h>
#include <bus/pci.h>
#include <util/string.h>

static void* s_initrd;
static size_t s_initrd_size;

extern void initrd_init(void* initrd, size_t initrd_size);

static void init_drivers()
{
  /* initialize ramdisk block device */
  ramdisk_init();
  if (s_initrd != NULL)
    ramdisk_install(s_initrd, s_initrd_size);

  /* initialize PCI and perform a PCI bus scan */
  pci_init();

  /* initialize the platform's device drivers. */
  platform_init_drivers();
}

static void init_task_func()
{
  debug(INIT, "welcome from the startup kernel task\n");

  delete_init_stack();
  irq_kernel_init();

  /* initialize the block device manager */
  blockdev_init();

  /* initialize device drivers */
  init_drivers();

  /* initialize the virtual file system in order
   * to mount the initial ramdisk. */
  vfs_init(cmdline_get("rootfs"));

  /* obtain the name of the init program to
   * run as PID 1, which is passed on the kernel
   * command line. */
  const char* init_program = cmdline_get("init");
  kpanic(init_program, "cmdline: no 'init' parameter given");
  int status = proc_start(init_program);
  if (status < 0)
  {
    debug(ASSERT, "%s: cannot start program: %s\n",
          init_program, strerror(-status));
    kpanic(false, "kernel startup failed");
  }
}

void kmain(const char *cmdline, void *initrd, size_t initrd_size)
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
  s_initrd = initrd;
  s_initrd_size = initrd_size;

  /* initialize the scheduler's data structures as well
   * as the task list used to keep track of all running
   * tasks. terminated tasks are periodically removed
   * from the list by the tl cleanup kernel task. */
  sched_init();

  /* crate the init kernel task, which performs further
   * initialization of the kernel. */
  task_t* init_task = create_kernel_task(init_task_func);
  sched_insert(init_task);

  /* enable the scheduler and yield(). this will
   * not return. */
  sched_enable();
}
