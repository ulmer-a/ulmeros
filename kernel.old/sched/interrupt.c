#include <arch.h>
#include <debug.h>
#include <interrupt.h>
#include <arch/context.h>

#define EXC_PAGEFAULT   14
#define EXC_YIELD       15

extern void page_fault(size_t error);
extern void schedule();

typedef struct
{
  int present;
  void (*handler)(void*);
  void* driver_data;
} handler_t;

static handler_t irq_handlers[16];
static uint64_t timer_ticks_;

void irq_init()
{
  arch_irq_init();
  for (int i = 0; i < 16; i++)
    irq_handlers[i].present = 0;
}

void interrupt(size_t irq)
{
  // further IRQ's are disabled!

  if (irq < 16)
  {
    handler_t* handler = irq_handlers + irq;
    if (handler->present)
    {
      handler->handler(handler->driver_data);
    }
  }

  //debug(IRQ, "unhandled IRQ #%zd\n", irq);
}

void exception(size_t exc)
{
  switch (exc)
  {
  case EXC_PAGEFAULT:
    page_fault(ctx_error(saved_context));
    return;
  case EXC_YIELD:
    schedule();
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

int irq_register(size_t irq, void (*func)(void*), void* drv)
{
  handler_t handler = {
    .present = 0,
    .handler = func,
    .driver_data = drv
  };
  irq_handlers[irq] = handler;
  irq_handlers[irq].present = 1;
  return irq;
}

void irq_unregister(int descriptor)
{
  irq_handlers[descriptor].present = 0;
}
