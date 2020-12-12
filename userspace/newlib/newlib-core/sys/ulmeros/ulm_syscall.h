#pragma once

#include "../../../../kernel/include/syscall-ids.h"

typedef unsigned long mw_t;

static inline mw_t SYSCALL_0(mw_t id,
                             mw_t arg1, mw_t arg2, mw_t arg3,
                             mw_t arg4, mw_t arg5, mw_t arg6)
{
  mw_t return_value;
  __asm__ volatile (
    "mov %0, %%rax;"
    "mov %1, %%rbx;"
    "mov %2, %%rcx;"
    "mov %3, %%rdx;"
    "mov %4, %%rsi;"
    "mov %5, %%rdi;"
    "mov %6, %%r8;"
    "int $0x80;"
    "mov %%rax, %0;"
    : "=r"(return_value)
    : "r"(id),
      "r"(arg1), "r"(arg2),  "r"(arg3),
      "r"(arg4), "r"(arg5),  "r"(arg6)
    : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8"
  );
  return return_value;
}
