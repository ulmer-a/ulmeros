#include <x86/ports.h>

void printdbg(const char* str)
{
  while (*str)
    outb(0xe9, *str++);
}
