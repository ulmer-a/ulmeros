#include <debug.h>
#include <arch/common.h>

void panic()
{
  preempt_disable();
  halt_core();
}
