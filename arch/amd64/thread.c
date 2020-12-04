#include <arch/context.h>
#include <memory.h>

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
  debug(ASSERT, "rip=%p, rsp=%p, err=%p\n",
        ctx->rip, ctx->rsp, ctx->error);
}

void ctx_set_kernel_stack(void* stack_ptr)
{
  set_rsp0(stack_ptr);
}

size_t ctx_irq(arch_context_t* ctx)
{
  return ctx->irq;
}

size_t ctx_error(arch_context_t* ctx)
{
  return ctx->error;
}
