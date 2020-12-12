#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

#include "ulm_syscall.h"

int open(const char *name, int flags, ...)
{
  long ret = SYSCALL_0(SYS_READ, (mw_t)name,
    (mw_t)flags, 0, 0, 0, 0);
  if (ret < 0)
  {
    errno = -ret;
    return -1;
  }
  return ret;
}

int read(int file, char *ptr, int len)
{
  long ret = SYSCALL_0(SYS_READ, (mw_t)file,
    (mw_t)ptr, (mw_t)len, 0, 0, 0);
  if (ret < 0)
  {
    errno = -ret;
    return -1;
  }
  return ret;
}

int write(int file, char *ptr, int len)
{
  long ret = SYSCALL_0(SYS_READ, (mw_t)file,
    (mw_t)ptr, (mw_t)len, 0, 0, 0);
  if (ret < 0)
  {
    errno = -ret;
    return -1;
  }
  return ret;
}

int lseek(int file, int ptr, int dir)
{
  errno = ENOSYS;
  return -1;
}

int close(int file)
{
  long ret = SYSCALL_0(SYS_CLOSE, (mw_t)file,
    0, 0, 0, 0, 0);
  if (ret < 0)
  {
    errno = -ret;
    return -1;
  }
  return ret;
}

int stat(const char *file, struct stat *st)
{
  (void)file; (void)st;
  errno = ENOSYS;
  return -1;
}

int fstat(int file, struct stat *st)
{
  (void)file; (void)st;
  errno = ENOSYS;
  return -1;
}

int link(char *old, char *new)
{
  (void)old; (void)new;
  errno = ENOSYS;
  return -1;
}

int unlink(char *name)
{
  (void)name;
  return -1;
}
