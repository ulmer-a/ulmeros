#include <arch/debug.h>
#include <ports.h>

void arch_print(char c)
{
  outb(0xe9, c);
}
