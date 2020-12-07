#include <types.h>
#include <kstring.h>
#include <stdarg.h>
#include <mutex.h>
#include <arch/debug.h>
#include <arch/context.h>

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


static char* debug_levels[] = {
  COL_BLUE    "VSPACE " COL_RESET,
  COL_WHITE   "KMAIN  " COL_RESET,
  COL_RED     "ASSERT " COL_RESET,
  COL_BLUE    "PAGEMGR" COL_RESET,
  COL_BLUE    "KHEAP  " COL_RESET,
  COL_CYAN    "IRQ    " COL_RESET,
  COL_BLUE    "PAGEFLT" COL_RESET,
  COL_MAGENTA "SCHED  " COL_RESET,
  COL_GREEN   "EXT2FS " COL_RESET,
  COL_GREEN   "VFS    " COL_RESET,
  COL_CYAN    "ATADISK" COL_RESET,
  COL_WHITE   "PCIBUS " COL_RESET,
  COL_GREEN   "BLKDEV " COL_RESET
};

static char kprintf_buffer[4096];
static mutex_t kprintf_mtx = MUTEX_INITIALIZER;

static void debug_write(char* buffer, size_t length)
{
  while (length--)
    arch_print(*buffer++);
}

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

  debug_write(kprintf_buffer, strlen(kprintf_buffer));
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
  ctx_dump(saved_context);
  debug(ASSERT, "\n\n--- PANIC ---\n");
  __asm__ volatile ("cli; hlt");
}
