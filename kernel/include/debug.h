#pragma once

#define OUTPUT_ENABLED BIT(31)

#define VSPACE      0   | OUTPUT_ENABLED
#define KMAIN       1   | OUTPUT_ENABLED
#define ASSERT      2   | OUTPUT_ENABLED
#define PAGEMGR     3   | OUTPUT_ENABLED
#define KHEAP       4   | OUTPUT_ENABLED
#define IRQ         5   | OUTPUT_ENABLED
#define PAGEFAULT   6   | OUTPUT_ENABLED
