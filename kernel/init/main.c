#include <types.h>
#include <kstring.h>
#include <vspace.h>
#include <memory.h>
#include <interrupt.h>
#include <task.h>
#include <arch.h>
#include <sched.h>

extern char _bss_start;
extern char _bss_end;

/* for some reason, I didn't manage to get function
 * pointers to the following functions to work. they
 * will always evaluate to zero. Setting the type to
 * 'char' instead of void(*)() is a workaround. */
extern char sysinit_task; // void sysinit()
extern char idle_task;    // void idle_task()

void kmain(boot_info_t* bootinfo)
{
  debug(KMAIN, "ULMER Operating System %s - built %s\n\n"
               "reached kmain()\n",
        OS_VERSION, OS_BUILD_DATE);

  /* clear BSS segment */
  memset(&_bss_start, 0, (size_t)&_bss_end - (size_t)&_bss_start);

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

  /* create the system initialization task and the
   * idle task and insert them into the scheduler. */
  create_ktask((void*)&sysinit_task);
  create_ktask((void*)&idle_task);

  /* start the scheduler, this will not return */
  sched_start();
}
