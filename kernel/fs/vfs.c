#include <types.h>
#include <errno.h>

ssize_t sys_read(char* buffer, size_t len)
{
  return -ENOSYS;
}

ssize_t sys_write(char* buffer, size_t len)
{
  return -ENOSYS;
}

int sys_open(char* path, int flags, int mode)
{
  return -ENOSYS;
}

int sys_close(int fd)
{
  return -ENOSYS;
}
