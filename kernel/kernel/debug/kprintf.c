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
  COL_YELLOW    "SYSCALL"
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
  printdbg(COL_RESET);
  va_end(args);
}
