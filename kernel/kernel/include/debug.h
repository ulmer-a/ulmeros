#pragma once

#include <util/types.h>

#define OUTPUT_ENABLED  BIT(31)

#define VSPACE      0   //| OUTPUT_ENABLED
#define INIT        1   | OUTPUT_ENABLED
#define ASSERT      2   | OUTPUT_ENABLED
#define PAGEMGR     3   //| OUTPUT_ENABLED
#define KHEAP       4   //| OUTPUT_ENABLED
#define IRQ         5   | OUTPUT_ENABLED
#define PAGEFAULT   6   | OUTPUT_ENABLED
#define SCHED       7   | OUTPUT_ENABLED
#define EXT2FS      8   | OUTPUT_ENABLED
#define VFS         9   | OUTPUT_ENABLED
#define ATADISK     10  //| OUTPUT_ENABLED
#define PCIBUS      11  | OUTPUT_ENABLED
#define BLKDEV      12  | OUTPUT_ENABLED
#define LOADER      13  | OUTPUT_ENABLED
#define TASK        14  | OUTPUT_ENABLED
#define TASKLIST    15  | OUTPUT_ENABLED
#define RAMFS       16  | OUTPUT_ENABLED
#define PROCESS     17  | OUTPUT_ENABLED
#define SYSCALL     18  | OUTPUT_ENABLED

extern void debug(unsigned level, const char* fmt, ...);
extern void panic();

#define kpanic(x, msg) if (!(x)) {                          \
  debug(ASSERT, "\n\n"                                      \
                "panic: %s() line %u (%s)\n"                \
                "error: %s\n",                              \
    __func__, __LINE__, __FILE__, msg);                     \
  panic();                                                  \
  }

#ifdef DEBUG
#define assert(x, msg) kpanic(x, "assert failed: " msg)
#else
#define assert(x, msg)
#endif
