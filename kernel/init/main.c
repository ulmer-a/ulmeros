#include <types.h>
#include <kstring.h>
#include <vspace.h>

extern char _bss_start;
extern char _bss_end;

void main64()
{
  // clear BSS
  memset(&_bss_start, 0, (size_t)&_bss_end - (size_t)&_bss_start);

  debug(KMAIN, "reached main64()\n");

  // initialize the kernel page tables
  vspace_init_kernel();

  // SETUP KERNEL HEAP

  // start the scheduler as FAST AS POSSIBLE

  // perform memory init

  // get some debug output

  // take care of interrupts

  // start the scheduler

  // start the intitialization kernel thread
}
