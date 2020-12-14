/* don't include anything! */

typedef unsigned long size_t;

size_t _syscall(size_t id, size_t arg1, size_t arg2,
                size_t arg3, size_t arg4, size_t arg5,
                size_t arg6)
{
  size_t return_value = (size_t)-1;
  __asm__ volatile (
    "mov %0, %%rax;"
    "mov %1, %%rbx;"
    "mov %2, %%rcx;"
    "mov %3, %%rdx;"
    "mov %4, %%rsi;"
    "mov %5, %%rdi;"
    "mov %6, %%r8;"
    "int $0x80;"
    "mov %%rax, %0"
    : "=r"(return_value)
    : "r"(id), "r"(arg1), "r"(arg2), "r"(arg3),
        "r"(arg4), "r"(arg5), "r"(arg6)
    : "%rax", "%rbx", "%rcx", "%rdx",
        "%rsi", "%rdi", "%r8"
  );
  return return_value;
}
