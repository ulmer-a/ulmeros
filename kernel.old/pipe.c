#include <syscalls.h>
#include <errno.h>

int sys_pipe(int* read_end, int* write_end)
{
  return -ENOSYS;
}
