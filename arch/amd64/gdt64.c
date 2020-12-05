/*
 * ULMER Operating System
 * Architecture initialization code for amd64
 * Copyright (C) 2019.
 * Written by Alexander Ulmer <ulmer@student.tugraz.at>
 */

#include <types.h>
#include <kstring.h>

#define PRESENT     BIT(15)             // selector is present
#define OPSIZE_32   BIT(22)             // default operand size 32bit
#define LONGMODE    BIT(21)             // enable long mode
#define DPL_USER    (BIT(31)|BIT(14))   // allow user mode
#define GRAN_4K     BIT(23)             // granularity = 4K
#define ACC_EXC_RD  BIT(9)              // execute & read

#define CODESEG     (BIT(11)|BIT(12))
#define DATASEG     (BIT(12))

// null, kcode, kdata
#define GDT_ENTRIES 5

/* global descriptor table descriptor */
typedef struct gdtd_struct
{
  uint16_t size;
  struct gdte_struct *addr;
} __attribute__((packed)) gdtd_t;

/* global descriptor table entry */
typedef struct gdte_struct
{
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t flags0;
  uint8_t flags1_limit;
  uint8_t base_high;
} __attribute__((packed)) gdte_t;

typedef struct tss_struct
{
  uint32_t reserved0;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved1;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved2;
  uint16_t reserved3;
  uint16_t iopb_offset;
} __attribute__((packed)) tss_t;

static gdtd_t gdt_descriptor;
static gdte_t gdt[GDT_ENTRIES];
static tss_t  tss;

extern char _bss_start;
extern char _bss_end;

static void setup_entry(int id, uint32_t base, uint32_t limit,
                        uint32_t flags)
{
  gdt[id].base_low    = base & 0xffff;
  gdt[id].base_middle = (base >> 16) & 0xff;
  gdt[id].base_high   = (base >> 24) & 0xff;

  gdt[id].limit = limit & 0xffff;
  gdt[id].flags1_limit = (limit >> 16) & 0b1111;

  *((uint32_t*)&(gdt[id].base_middle)) |= flags;
}

void gdt_init(void)
{
  gdt_descriptor.size = sizeof(gdte_t) * GDT_ENTRIES - 1;
  gdt_descriptor.addr = gdt;

  setup_entry(0, 0, 0xf0000, GRAN_4K); // null descriptor required by design

  setup_entry(1, 0, 0xf0000, PRESENT | GRAN_4K | LONGMODE | ACC_EXC_RD | CODESEG);  // kcode
  setup_entry(2, 0, 0xf0000, PRESENT | GRAN_4K | LONGMODE | ACC_EXC_RD | DATASEG);  // kdata
  setup_entry(3, 0, 0xf0000, PRESENT | GRAN_4K | LONGMODE | ACC_EXC_RD | CODESEG | DPL_USER);  // ucode
  setup_entry(4, 0, 0xf0000, PRESENT | GRAN_4K | LONGMODE | ACC_EXC_RD | DATASEG | DPL_USER);  // udata


  __asm__ volatile("lgdt %0" : : "g"(gdt_descriptor));
}

void arch_init()
{
  /* clear BSS segment */
  memset(&_bss_start, 0, (size_t)&_bss_end - (size_t)&_bss_start);

  gdt_init();
}

void set_rsp0(void* rsp)
{
  tss.rsp0 = (size_t)rsp;
}
