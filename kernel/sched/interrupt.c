#include <arch.h>
#include <debug.h>
#include <arch/context.h>

#define EXC_PAGEFAULT   14

extern void page_fault(size_t error);

void interrupt(size_t irq)
{
  debug(IRQ, "IRQ #%zd\n", irq);
}

void exception(size_t exc)
{
  switch (exc)
  {
  case EXC_PAGEFAULT:
    page_fault(ctx_error(saved_context_));
    break;
  }

  debug(IRQ, "CPU exception #%zd\n", exc);
  assert(false, "CPU EXCEPTION");
}
