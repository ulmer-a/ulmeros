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
  return SYSCALL_0(SYS_GETPID, 0, 0, 0, 0, 0, 0);
}

int fork()
{
  long ret = SYSCALL_0(SYS_FORK, 0, 0, 0, 0, 0, 0);
  if (ret < 0)
  {
    errno = -ret;
    return -1;
  }
  return ret;
}

int execve(char *name, char **argv, char **env)
{
  long ret = SYSCALL_0(SYS_FORK, (mw_t)name,
    (mw_t)argv, (mw_t)env, 0, 0, 0);
  if (ret < 0)
  {
    errno = -ret;
    return -1;
  }
  return ret;
}

int wait(int *status)
{
  (void)status;
  errno = ENOSYS;
  return -1;
}

int kill(int pid, int sig)
{
  long ret = SYSCALL_0(SYS_FORK, (mw_t)pid,
    (mw_t)sig, 0, 0, 0, 0);
  if (ret < 0)
  {
    errno = -ret;
    return -1;
  }
  return ret;
}

void _exit(int status)
{
  SYSCALL_0(SYS_EXIT, 0, 0, 0, 0, 0, 0);
  for(;;);
}

int isatty(int file)
{
  errno = ENOSYS;
  return -1;
}

caddr_t sbrk(int incr)
{
  errno = ENOSYS;
  return (void*)-1;
}

clock_t times(struct tms *buf)
{
  (void)buf;
  errno = ENOSYS;
  return -1;
}

int gettimeofday(struct timeval *p, void *z)
{
  (void)p; (void)z;
  errno = ENOSYS;
  return -1;
}
