#include <debug.h>
#include <arch/common.h>
#include <util/stdarg.h>
#include <util/string.h>
#include <sched/mutex.h>
#include <sched/interrupt.h>

static char kprintf_buffer[2048];
static mutex_t kprintf_buffer_lock = MUTEX_INITIALIZER;

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

const char* debug_type[] = {
  COL_MAGENTA   " VSPACE",
  COL_WHITE     "   INIT",
  COL_WHITE     " ASSERT",
  COL_MAGENTA   "PAGEMGR",
  COL_MAGENTA   "  KHEAP",
  COL_RED       "    IRQ",
  COL_GREEN     "PAGEFLT",
  COL_YELLOW    "  SCHED",
  COL_BLUE      "FILESYS",
  COL_BLUE      "VIRT_FS",
  COL_BLUE      "   DISK",
  COL_CYAN      " PCIBUS",
  COL_BLUE      " BLKDEV",
  COL_GREEN     " LOADER",
  COL_YELLOW    "   TASK",
  COL_YELLOW    "TASKMGR",
  COL_BLUE      "FILESYS",
  COL_YELLOW    "PROCESS",
  COL_YELLOW    "SYSCALL",
  COL_GREEN     " VSPACE"
};


void debug(unsigned level, const char *fmt, ...)
{
  if ((level & OUTPUT_ENABLED) == 0)
    return;

  if (irq_ongoing)
  {
    if (kprintf_buffer_lock.lock)
      return;
  }
  else
  {
    mutex_lock(&kprintf_buffer_lock);
  }

  sprintf(kprintf_buffer, "[%s]: ", debug_type[level & 0xff]);
  printdbg(kprintf_buffer);

  va_list args;
  va_start(args, fmt);

  _sprintf(kprintf_buffer, fmt, args);
  printdbg(kprintf_buffer);
  printdbg(COL_RESET);
  va_end(args);

  if (!irq_ongoing)
    mutex_unlock(&kprintf_buffer_lock);
}
