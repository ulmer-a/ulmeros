#include <debug.h>
#include <arch/common.h>
#include <util/stdarg.h>
#include <util/string.h>

static char kprintf_buffer[2048];

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
  COL_MAGENTA   " VSPACE" COL_RESET,
  COL_WHITE     "   INIT" COL_RESET,
  COL_WHITE     " ASSERT" COL_RESET,
  COL_MAGENTA   "PAGEMGR" COL_RESET,
  COL_MAGENTA   "  KHEAP" COL_RESET,
  COL_GREEN     "    IRQ" COL_RESET,
  COL_MAGENTA   "PAGEFLT" COL_RESET,
  COL_YELLOW    "  SCHED" COL_RESET,
  COL_BLUE      "FILESYS" COL_RESET,
  COL_BLUE      "VIRT_FS" COL_RESET,
  COL_BLUE      "   DISK" COL_RESET,
  COL_CYAN      " PCIBUS" COL_RESET,
  COL_BLUE      " BLKDEV" COL_RESET,
  COL_YELLOW    " LOADER" COL_RESET,
  COL_YELLOW    "   TASK" COL_RESET,
  COL_YELLOW    "TASKMGR" COL_RESET,
  COL_BLUE      "FILESYS" COL_RESET,
  COL_YELLOW    "PROCESS" COL_RESET
};

void debug(unsigned level, const char *fmt, ...)
{
  if ((level & OUTPUT_ENABLED) == 0)
    return;

  sprintf(kprintf_buffer, "[%s]: ", debug_type[level & 0xff]);
  printdbg(kprintf_buffer);

  va_list args;
  va_start(args, fmt);

  _sprintf(kprintf_buffer, fmt, args);
  printdbg(kprintf_buffer);
  va_end(args);
}
