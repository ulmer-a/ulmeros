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

size_t atomic_add(size_t* mem, ssize_t increment)
{
  size_t ret = increment;
  __asm__ __volatile__(
    "lock; xadd %0, %1;"
    : "=a" (increment), "=m"(*mem)
    : "a" (increment));
  return ret;
}
