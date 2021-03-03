#include <util/string.h>
#include <sched/task.h>
#include <sched/interrupt.h>
#include <x86/context.h>
#include <x86/ports.h>
#include <debug.h>


extern int page_fault(size_t address, int present,
                       int write, int user, int exec);

extern context_t* schedule(context_t* ctx);

#define EXC_GENERAL_PROT_FAULT  13
#define EXC_PAGE_FAULT          14
#define EXC_YIELD               31

int x86_exception(size_t exc, size_t error)
{
  debug(IRQ, "Processor exception #%zu (%s)\n", exc, strexcept(exc));
  debug(IRQ, "Error code: 0x%zx\n", error);

  switch (exc)
  {

  case EXC_PAGE_FAULT: {
    size_t addr = 0;
    __asm__ volatile ("mov %%cr2, %0;" : "=r"(addr));
    return page_fault(addr,
      error & BIT(0) ? 1 : 0,
      error & BIT(1) ? 1 : 0,
      error & BIT(2) ? 1 : 0,
      error & BIT(4) ? 1 : 0
    );
  }

  default:
    debug(IRQ, "Unhandled exception\n");
    return false;
  }

  return false;
}

extern void irq_handler(size_t id);

typedef struct
{
  size_t baseptr;
  size_t ret_addr;
} stack_frame_t;

static void backtrace(size_t baseptr)
{
  int i = 5;
  debug(IRQ, "-- Backtrace:\n");
  while (i--)
  {
    stack_frame_t* sf = (stack_frame_t*)baseptr;
    debug(IRQ, "  %p\n", sf->ret_addr);
    baseptr = sf->baseptr;
  }
}

context_t* x86_irq_handler(context_t* ctx)
{
  if (ctx->irq < 32)
  {
    if (ctx->irq == EXC_YIELD)
    {
      /* someone called yield() */
      ctx = schedule(ctx);
    }
    else
    {
      preempt_enable();
      if (!x86_exception(ctx->irq, ctx->error))
      {
        debug(IRQ, "rip=%p, rsp=%p, rdi=%p, rsi=%p\n"
                   "rax=%p, rbx=%p, rcx=%p, rdx=%p\n",
              ctx->rip, ctx->rsp, ctx->rdi, ctx->rsi,
              ctx->rax, ctx->rbx, ctx->rcx, ctx->rdx);
        backtrace(ctx->rbp);
        assert(false, "Unhandled exception\n");
      }
      preempt_disable();
    }
  }
  else if (ctx->irq < 48)
  {
    size_t irq_id = ctx->irq - 32;
    /* irq handlers run with further
     * interrupts disabled. */

    irq_ongoing = true;
    irq_handler(irq_id);
    irq_ongoing = false;

    if (irq_id == 0)
    {
      /* when the timer interrupt fires, run
       * the scheduler. */
      ctx = schedule(ctx);
    }

    /* notify IRQ controller that IRQ was handled. */
    if (irq_id >= 8)
      outb(0xa0, 0x20);
    outb(0x20, 0x20);
  }
  else
  {
    assert(false, "unknown IRQ source");
  }

  return ctx;
}

void yield()
{
  /* yielding the time slice will trigger
   * interrupt 0x1f. this will run schedule()
   * in an interrupt context but without
   * the timer being fired. */
  __asm__ volatile("int $0x1f;");
}
