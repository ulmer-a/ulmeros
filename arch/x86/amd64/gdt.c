/*
 * UlmerOS x86_64 Global descriptor table and Task state segment
 * Copyright (C) 2017-2021 Alexander Ulmer
 *
 * The GDT configuration provides six different segment
 * selectors that can be used. With compatibility mode
 * user segment selectors, the operating system can also
 * run 32bit x86 programs in compatibility mode.
 *
 * Usable segment selector values
 *  0x08 -> Kernel code segment
 *  0x10 -> Kernel data segment
 *  0x18 -> Long mode user code segment
 *  0x20 -> Long mode user data segment
 *  0x28 -> Compatibility mode user code segment
 *  0x30 -> Compatibility mode user data segment
 */

#include <util/types.h>
#include <mm/memory.h>
#include <debug.h>

#define GDT_ENTRIES     7

/* segment descriptor flags. they can be combined to
 * create different variations of segment descriptors. */
#define PRESENT         BIT(47)             // selector is present
#define CODE            (BIT(43)|BIT(44))   // code segment
#define CODE_R          BIT(41)             // code segment readable
#define DATA            BIT(44)             // data segment
#define DATA_W          BIT(41)             // data segment writable
#define LONGMODE        BIT(53)             // long mode / compatibility mode
#define DPL_USER        (BIT(45)|BIT(46))   // user mode
#define MAX_LIMIT_VAL   0xf00000000ffff     // maximum segment limit
#define GRANULARITY     BIT(55)             // set granularity to 4K
#define MAX_LIMIT       MAX_LIMIT_VAL|GRANULARITY
#define OPSIZE32        BIT(54)             // set compat. op size to 32bit

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
  s_gdt_descriptor.size = sizeof(uint32_t) * GDT_ENTRIES;
  s_gdt_descriptor.addr = s_gdt;
  __asm__ volatile("lgdt %0" : : "g"(s_gdt_descriptor));
}

void setup_gdt()
{
  s_gdt[0] = 0;

  /* 64bit long mode kernel segment descriptors */
  s_gdt[1] = PRESENT | LONGMODE | MAX_LIMIT | CODE | CODE_R;
  s_gdt[2] = PRESENT | LONGMODE | MAX_LIMIT | DATA | DATA_W;

  /* 64bit long mode user segment descriptors*/
  s_gdt[3] = PRESENT | LONGMODE | MAX_LIMIT | CODE | CODE_R | DPL_USER;
  s_gdt[4] = PRESENT | LONGMODE | MAX_LIMIT | DATA | DATA_W | DPL_USER;

  /* 32bit compatibility mode user segment descriptors */
  s_gdt[5] = PRESENT | OPSIZE32 | MAX_LIMIT | CODE | CODE_R | DPL_USER;
  s_gdt[6] = PRESENT | OPSIZE32 | MAX_LIMIT | DATA | DATA_W | DPL_USER;

  load_gdt();
}
