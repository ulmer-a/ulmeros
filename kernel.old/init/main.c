#include <types.h>
#include <kstring.h>
#include <vspace.h>
#include <memory.h>
#include <interrupt.h>
#include <task.h>
#include <arch.h>
#include <sched.h>

extern void sysinit_task();
static void start_sysinit_task()
{
  sysinit_task();
  for(;;) __asm__ volatile ("hlt");
}

void kmain(boot_info_t* bootinfo)
{
  debug(KMAIN, "ULMER Operating System %s - built %s\n\n",
        OS_VERSION, OS_BUILD_DATE);
  debug(KMAIN, "reached kmain()\n");

  /* allocate the free pages refcounter and perform
   * a memory scan and mark the pages used by the
   * kernel and the boot32 heap as reserved. */
  page_init(bootinfo);

  /* read the kernel page tables and save pointers to
   * them. this will enable vspace_init() to map the
   * kernel above the user break. */
  vspace_init_kernel();

  /* initialize the interrupt handlers and the
   * system timer */
  irq_init();
  timer_reset(50);

  /* initialize the kernel heap. this will create a
   * mapping at the very top of the address space */
  kheap_init();

  /* initialize the scheduler. this will allocate
   * the task list and perform some initialization. */
  sched_init();

  /* create the system initialization task and
   * insert it into the scheduler. */
  create_ktask(start_sysinit_task);

  /* start the scheduler, this will not return */
  sched_start();
}
