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

static void sysinit_task()
{
  debug(KMAIN, "Welcome from the first kernel task!\n");
}

static void idle_task()
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
  //create_ktask(sysinit_task);
  create_ktask(idle_task);

  /* start the scheduler */
  sched_start();
}
