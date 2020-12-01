#include <types.h>
#include <kstring.h>
#include <stdarg.h>
#include <mutex.h>
#include <arch/serial.h>

static char* debug_levels[] = {
  "VSPACE ",
  "KMAIN  ",
  "ASSERT ",
  "PAGEMGR",
  "KHEAP  ",
  "IRQ    ",
  "PAGEFLT",
  "SCHED  "
};

static char kprintf_buffer[4096];
static mutex_t kprintf_mtx = MUTEX_INITIALIZER;

static void do_kprintf(int level, const char *fmt, va_list args)
{
  mutex_lock(&kprintf_mtx);

  char conv_buffer[256];
  char *buffer_ptr = kprintf_buffer;

  char* debug_level = debug_levels[level];
  *buffer_ptr++ = '[';
  while (*debug_level)
    *buffer_ptr++ = *debug_level++;
  *buffer_ptr++ = ']';
  *buffer_ptr++ = ' ';

  while (*fmt)
  {
    char c = *fmt++;

    if (c == '%')
    {
      char *to_print;
      char modifier;
      char type = *fmt++;

      if (type == 'z' || type == 'l')
      {
        modifier = type;
        type = *fmt++;
      }

      if (type == 0)
        break;

      if (type == 's')
      {
        to_print = va_arg(args, char*);
      }
      else if (type == 'p')
      {
        ptoa(va_arg(args, void*), conv_buffer);
        to_print = conv_buffer;
      }
      else if (type == 'd')
      {
        if (modifier == 'l')
          ltoa(va_arg(args, int), conv_buffer, 10);
        else if (modifier == 'z')
          ultoa(va_arg(args, int), conv_buffer, 10);
        else
          itoa(va_arg(args, int), conv_buffer, 10);
        to_print = conv_buffer;
      }
      else if (type == 'x')
      {
        if (modifier == 'l')
          ltoa(va_arg(args, int), conv_buffer, 16);
        else if (modifier == 'z')
          ultoa(va_arg(args, int), conv_buffer, 16);
        else
          itoa(va_arg(args, int), conv_buffer, 16);
        to_print = conv_buffer;
      }
      else if (type == 'u')
      {
        utoa(va_arg(args, int), conv_buffer, 10);
        to_print = conv_buffer;
      }

      while (*to_print)
        *buffer_ptr++ = *to_print++;
    }
    else
    {
      *buffer_ptr++ = c;
    }
  }
  *buffer_ptr = 0;

  serial0_write(kprintf_buffer, strlen(kprintf_buffer));
  mutex_unlock(&kprintf_mtx);
}

void debug(int level, const char *fmt, ...)
{
  if ((level & OUTPUT_ENABLED) == 0)
    return;

  va_list args;
  va_start(args, fmt);
  do_kprintf(level & 0xff, fmt, args);
  va_end(args);
}

void panic()
{
  debug(ASSERT, "\n\n--- PANIC ---\n");
  __asm__ volatile ("cli; hlt");
}
