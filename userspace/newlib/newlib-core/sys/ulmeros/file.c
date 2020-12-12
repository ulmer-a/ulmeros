#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

int open(const char *name, int flags, ...)
{
  errno = ENOSYS;
  return -1;
}

int read(int file, char *ptr, int len)
{
  errno = ENOSYS;
  return -1;
}

int write(int file, char *ptr, int len)
{
  errno = ENOSYS;
  return -1;
}

int lseek(int file, int ptr, int dir)
{
  errno = ENOSYS;
  return -1;
}

int close(int file)
{
  errno = ENOSYS;
  return -1;
}

int stat(const char *file, struct stat *st)
{
  errno = ENOSYS;
  return -1;
}

int fstat(int file, struct stat *st)
{
  errno = ENOSYS;
  return -1;
}

int link(char *old, char *new)
{
  errno = ENOSYS;
  return -1;
}

int unlink(char *name)
{
  return -1;
}
