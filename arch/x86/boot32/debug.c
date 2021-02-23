#include <util/string.h>
#include <util/stdarg.h>

#include "boot32.h"

#define COL_RESET   "\x1b[0m"
#define COL_RED     "\x1b[31m"
#define COL_BLACK   "\x1b[30m"
#define COL_RED     "\x1b[31m"
#define COL_GREEN   "\x1b[32m"
#define COL_YELLOW  "\x1b[33m"
#define COL_BLUE    "\x1b[34m"
#define COL_MAGENTA "\x1b[35m"
#define COL_CYAN    "\x1b[36m"
#define COL_WHITE   "\x1b[37m"

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
