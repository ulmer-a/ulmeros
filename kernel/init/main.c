#include <types.h>
#include <kstring.h>

extern char _bss_start;
extern char _bss_end;

void main64()
{
  // clear BSS
  memset(&_bss_start, 0, (size_t)&_bss_end - (size_t)&_bss_start);

  debug("[ INFO ] reached main64()\n");

  // start the scheduler as FAST AS POSSIBLE

  // perform memory init

  // get some debug output

  // take care of interrupts

  // start the scheduler

  // start the intitialization kernel thread
}
