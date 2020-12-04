#include <arch/debug.h>
#include <amd64/ports.h>

void arch_print(char c)
{
  outb(0xe9, c);
}
