#include <util/string.h>
#include <sched/task.h>
#include <sched/interrupt.h>
#include <x86/ports.h>
#include <x86/context.h>
#include <debug.h>
#include <syscalls.h>
#include <errno.h>

extern int page_fault(size_t address, int present,
                       int write, int user, int exec);

extern context_t* schedule(context_t* ctx);

#define EXC_GENERAL_PROT_FAULT  13
#define EXC_PAGE_FAULT          14
#define EXC_YIELD               31

int x86_exception(size_t exc, size_t error)
{
  debug(IRQ, "Processor exception #%zu (%s), error=0x%zx\n",
             exc, strexcept(exc), error);

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

context_t* x86_irq_handler(context_t* ctx)
{
  if (ctx->irq == 0x80)
  {
    const size_t sys_count = syscall_count();
    if (ctx->error >= sys_count)
    {
      /* if the system call id is invalid,
       * report ENOSYS error and return  */
      #ifdef ARCH_X86_64
        ctx->rax = -ENOSYS;
      #else
        ctx->eax = -ENOSYS;
      #endif
      return ctx;
    }

    /* call the corresponding syscall routine */
    #ifdef ARCH_X86_64
      size_t (*sys_func)(size_t a1, size_t a2, size_t a3,
          size_t a4, size_t a5, size_t a6) = syscall_table[ctx->rax];
      ctx->rax = sys_func(
        ctx->r8,  ctx->r9,  ctx->r10,
        ctx->r11, ctx->r12, ctx->r13
      );
    #else
      size_t (*sys_func)(size_t a1, size_t a2, size_t a3,
          size_t a4, size_t a5) = syscall_table[ctx->eax];
      ctx->eax = sys_func(
        ctx->ebx,  ctx->ecx,  ctx->edx,
        ctx->esi, ctx->edi
      );
    #endif
  }
  else if (ctx->irq < 32)
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

void idle()
{
  __asm__ volatile ("hlt");
}
