#include <sched.h>
#include <types.h>

size_t xchg(size_t val, size_t* mem)
{
  __asm__ volatile ("xchg %0, %1"
    : "=r" (val)
    : "m" (*mem), "0"(val)
    : "memory");
  return val;
}
