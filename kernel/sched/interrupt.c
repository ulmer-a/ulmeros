#include <arch.h>
#include <debug.h>
#include <interrupt.h>
#include <arch/context.h>

#define EXC_PAGEFAULT   14

extern void page_fault(size_t error);
extern void schedule();

static void* irq_handlers[16];
static uint64_t timer_ticks_;

void irq_init()
{
  arch_irq_init();
  for (int i = 0; i < 16; i++)
    irq_handlers[i] = NULL;
}

void interrupt(size_t irq)
{
  if (irq < 16)
  {
    if (irq_handlers[irq])
    {
      ((void (*)())irq_handlers[irq])();
      return;
    }
  }

  debug(IRQ, "unhandled IRQ #%zd\n", irq);
}

void exception(size_t exc)
{
  switch (exc)
  {
  case EXC_PAGEFAULT:
    page_fault(ctx_error(saved_context_));
    return;
  }

  debug(IRQ, "CPU exception #%zd\n", exc);
  assert(false, "CPU EXCEPTION");
}

void timer()
{
  timer_ticks_++;
  schedule();
}

uint64_t timer_ticks()
{
  return timer_ticks_;
}

void irq_install_handler(unsigned irq, void (*handler)(void))
{
  assert(irq < 16, "invalid irq number!");

  irq_handlers[irq] = handler;
}

void irq_uninstall_handler(unsigned irq)
{
  irq_install_handler(irq, NULL);
}
