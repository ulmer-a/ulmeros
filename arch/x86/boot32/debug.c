#include <util/string.h>
#include <util/stdarg.h>

#include "boot32.h"

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

static char debug_buffer[1024];

static void flush()
{
  const char *str = debug_buffer;
  while (*str)
    outb(0xe9, *str++);
}

void debug(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  _sprintf(debug_buffer, fmt, args);
  flush();

  va_end(args);
}

void _assert(const char* function, const char* file, int line, const char *msg)
{
  debug("assert failed: function %s in %s on line %d:\n%s\n",
        function, file, line, msg);
  halt_core();
}
