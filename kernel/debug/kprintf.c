#include <debug.h>
#include <arch/common.h>
#include <util/stdarg.h>
#include <util/string.h>

static char kprintf_buffer[2048];

void debug(int level, const char *fmt, ...)
{
  if ((level & OUTPUT_ENABLED) == 0)
    return;

  va_list args;
  va_start(args, fmt);
  _sprintf(kprintf_buffer, fmt, args);
  printdbg(kprintf_buffer);
  va_end(args);
}
