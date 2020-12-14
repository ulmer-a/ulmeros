#include <unistd.h>
#include <errno.h>

#include "syscall.h"

void* sbrk(intptr_t increment)
{
  void* ret = (void*)_syscall(SYSNO_SBRK, increment);
  if (ret == (void*)-1)
    errno = ENOMEM;
  return ret;
}

void _exit(int status)
{
  _syscall(SYSNO_EXIT, status);
}
