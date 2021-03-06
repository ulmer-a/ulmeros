#include "boot32.h"

#define GDT_ENTRIES     3

/* segment descriptor flags. they can be combined to
 * create different variations of segment descriptors. */
#define PRESENT         BIT(47)             // selector is present
#define CODE            BIT(43) | BIT(44)   // code segment
#define CODE_R          BIT(41)             // code segment readable
#define DATA            BIT(44)             // data segment
#define DATA_W          BIT(41)             // data segment writable
#define LONGMODE        BIT(53)             // long mode / compatibility mode
#define DPL_USER        BIT(45) | BIT(46)   // user mode
#define OPSIZE32        BIT(54)             // set compat. op size to 32bit
#define MAX_LIMIT       0x8f00000000ffff    // maximum segment limit´

typedef struct
{
  uint16_t size;
  uint64_t* addr;
} __attribute__((packed)) gdt_t;

static gdt_t s_gdt_descriptor;
static uint64_t s_gdt[GDT_ENTRIES];

static void load_gdt()
{
  /* tell the CPU the location of the GDT by writing it's
   * size and location to the hidden GDT register */
  s_gdt_descriptor.size = sizeof(uint64_t) * GDT_ENTRIES;
  s_gdt_descriptor.addr = s_gdt;
  __asm__ volatile("lgdt %0" : : "g"(s_gdt_descriptor));
}

void gdt_long_init()
{
  s_gdt[0] = 0;

  /* 64bit long mode kernel segment descriptors */
  s_gdt[1] = PRESENT | LONGMODE | MAX_LIMIT | CODE | CODE_R;
  s_gdt[2] = PRESENT | LONGMODE | MAX_LIMIT | DATA | DATA_W;

  /* load the global descriptor table */
  load_gdt();
}
