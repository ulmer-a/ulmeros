#include <syscalls.h>

void* syscall_table[] = {
  NULL,
  sys_exit,
  sys_read
};

size_t syscall_count()
{
  return sizeof(syscall_table) / sizeof(void*);
}
