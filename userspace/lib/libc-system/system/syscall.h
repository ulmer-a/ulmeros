#pragma once

#include <sys/types.h>
#include "../../../../kernel/include/syscall-no.h"

extern size_t _syscall(size_t id, ...);

static inline size_t _sys_set_errno(size_t ret)
{
  ssize_t signed_ret = (ssize_t)ret;
  if (signed_ret < 0)
  {
    errno = -signed_ret;
    return -1;
  }
  return ret;
}
