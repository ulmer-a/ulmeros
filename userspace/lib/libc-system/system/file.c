#include <unistd.h>
#include <errno.h>

#include "syscall.h"

ssize_t read(int fd, char *buffer, size_t len)
{
  return (ssize_t)_sys_set_errno(_syscall(SYSNO_READ, fd, buffer, len));
}

ssize_t write(int fd, char *buffer, size_t len)
{
  return (ssize_t)_sys_set_errno(_syscall(SYSNO_WRITE, fd, buffer, len));
}

int close(int fd)
{
  return (int)_sys_set_errno(_syscall(SYSNO_CLOSE, fd));
}
