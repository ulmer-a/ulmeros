#include <syscalls.h>

void* syscall_table[] = {
  sys_exit,       // 0
  sys_read,       // 1
  sys_write,      // 2
  sys_open,       // 3
  sys_close,      // 4
  sys_fork,       // 5
  sys_exec,       // 6
  sys_wait,       // 7
  sys_getpid,     // 8
  sys_pipe,       // 9
  NULL
};
