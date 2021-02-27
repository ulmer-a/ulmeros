#include <mm/memory.h>
#include <arch/context.h>
#include <x86/context.h>
#include <x86/ports.h>


#define CS_KERNEL   0x08
#define DS_KERNEL   0x10
#define CS_USER     0x18
#define DS_USER     0x20

context_t* context_init(void* kstack_ptr, void* entry_addr, void* stack_ptr,
                        int user, size_t arg0, size_t arg1)
{
  context_t* ctx = (context_t*)kstack_ptr - 1;
  ctx->rsp = (size_t)stack_ptr;
  ctx->rip = (size_t)entry_addr;

  ctx->rdi = arg0;
  ctx->rsi = arg1;

  if (user)
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

  ctx->rflags = BIT(9);
  return ctx;
}

void context_set_pc(context_t* ctx, size_t pc)
{
  ctx->rip = pc;
}

void context_set_ret(context_t* ctx, size_t ret)
{
  ctx->rax = ret;
}
