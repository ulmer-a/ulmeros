#include <arch/context.h>
#include <memory.h>
#include "ports.h"

#include "amd64.h"

#define CS_KERNEL   0x08
#define DS_KERNEL   0x10
#define CS_USER     0x18
#define DS_USER     0x20

struct arch_context_
{
    size_t r15;
    size_t r14;
    size_t r13;
    size_t r12;
    size_t r11;
    size_t r10;
    size_t r9;
    size_t r8;
    size_t rdi;
    size_t rsi;
    size_t rdx;
    size_t rcx;
    size_t rbx;
    size_t rax;
    size_t rbp;
    size_t gs;
    size_t fs;
    size_t irq;
    size_t error;
    size_t rip;
    size_t cs;
    size_t rflags;
    size_t rsp;
    size_t ss;
} __attribute__((packed));

void arch_yield()
{
  __asm__ volatile ("int $15;");
}

void preempt_disable()
{
  cli();
}

void preempt_enable()
{
  sti();
}

arch_context_t* ctx_create(void *entry, void* kstack, void *stack, int flags)
{
  arch_context_t* ctx = (arch_context_t*)kstack - 1;
  ctx->rsp = (size_t)stack;
  ctx->rip = (size_t)entry;

  if (flags & CTX_USER)
  {
    ctx->cs = CS_USER;
    ctx->ss = DS_USER;
    ctx->fs = DS_USER;
    ctx->gs = DS_USER;
  }
  else
  {
    ctx->cs = CS_KERNEL;
    ctx->ss = DS_KERNEL;
    ctx->fs = DS_KERNEL;
    ctx->gs = DS_KERNEL;
  }

  // enable interrupts
  ctx->rflags = BIT(9);
  return ctx;
}

void ctx_dump(arch_context_t* ctx)
{
  debug(ASSERT, "rip=%p, rsp=%p, rbp=%p, err=%p\n"
      "          rdi=%p, rsi=%p, rax=%p, rbx=%p\n"
      "           fs=%p,  gs=%p, \n",
        ctx->rip, ctx->rsp, ctx->rbp, ctx->error,
        ctx->rdi, ctx->rsi, ctx->rax, ctx->rbx,
        ctx->fs,  ctx->gs);

  uint64_t* rsp = (uint64_t*)ctx->rsp;
  debug(ASSERT, "values at RSP:\n"
                "RSP-8:  %p\n"
                "RSP:    %p\n"
                "RSP+8:  %p\n"
                "RSP+16: %p\n",
        *(rsp-1), *rsp, *(rsp+1), *(rsp+2));
}

void ctx_set_kernel_stack(void* stack_ptr)
{
  set_rsp0(stack_ptr);
}

void ctx_check_seg(arch_context_t* ctx)
{
  if (ctx->fs > 0x40 || ctx->gs > 0x40)
  {
    assert(false, "segment registers invalid");
  }
}

size_t ctx_irq(arch_context_t* ctx)
{
  return ctx->irq;
}

size_t ctx_error(arch_context_t* ctx)
{
  return ctx->error;
}
