#include <util/types.h>

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
  __asm__ volatile(
          "lock xadd %0, (%1);"
          : "=a"(increment)
          : "r"(mem), "a"(increment)
          : "memory");
  return increment;
}