#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

#include "ulm_syscall.h"

int getpid()
{
  errno = ENOSYS;
  return -1;
}

int fork()
{
  errno = ENOSYS;
  return -1;
}

int execve(char *name, char **argv, char **env)
{
  errno = ENOSYS;
  return -1;
}

int wait(int *status)
{
  errno = ENOSYS;
  return -1;
}

int kill(int pid, int sig)
{
  errno = ENOSYS;
  return -1;
}

void _exit(int status)
{
  errno = ENOSYS;
  return -1;
}

int isatty(int file)
{
  errno = ENOSYS;
  return -1;
}

caddr_t sbrk(int incr)
{
  errno = ENOSYS;
  return -1;
}

clock_t times(struct tms *buf)
{
  errno = ENOSYS;
  return -1;
}

int gettimeofday(struct timeval *p, void *z)
{
  errno = ENOSYS;
  return -1;
}
