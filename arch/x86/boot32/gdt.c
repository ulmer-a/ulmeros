/*
 * GDT - Global Descriptor Table
 * Copyright (C) 2018-2019
 * Written by Alexander Ulmer
 *
 * Dec 2018:    first implementation
 * May 2019:    refactoring formatting, adding init section
 * Feb 2021:    refactoring, code improvements
 *
 * setup_gdt() sets up segmentation as one of the
 * very first steps during system boot. The UlmerOS
 * GDT contains 6 entries (segments):
 *  - mandatory null-entry
 *  - kernel code segment
 *  - kernel data segment
 *  - user code segment
 *  - user data segment
 *  - task state segment (used to store interrupt stack pointer)
 *
 * UlmerOS doesn't make use of segmentation, however,
 * the i386 architecture demands setting up a GDT.
 * All the segments are set in such a way that a
 * flat 4GB address space is the result. The only
 * fields used in the Task State Segment are ss0
 * (constant) and esp0. Whenever a context switch
 * occurs, esp0 becomes the kernel stack pointer.
 * update_tss(sp) sets that value.
 */

#include "boot32.h"

#define VALID   0x80
#define OFF_32  0x40
#define OFF_16  0x00
#define SYS_NON 0x10
#define SEG_SYS 0x00
#define ACC_SUP 0x00
#define ACC_USR 0x60
#define GRAN_4K 0x80
#define GRAN_BT 0x00
#define CXR     0x0A
#define DRW     0x02

// null, kcode, kdata, tss
#define GDT_ENTRIES 6

typedef struct gdtd_struct
{
  uint16_t size;
  struct gdte_struct *addr;
} __attribute__((packed)) gdtd_t;

typedef struct gdte_struct
{
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t flags;
  uint8_t base_high;
} __attribute__((packed)) gdte_t;

typedef struct tsse_struct
{
  uint32_t prev_tss;   // for hardware multitasking
  uint32_t esp0;       // kernel SP
  uint32_t ss0;        // kernel SS
  uint32_t esp1;
  uint32_t ss1;
  uint32_t esp2;
  uint32_t ss2;
  uint32_t cr3;
  uint32_t eip;
  uint32_t eflags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint32_t es;         // kernel ES
  uint32_t cs;         // kernel CS
  uint32_t ss;         // kernel SS
  uint32_t ds;         // kernel DS
  uint32_t fs;         // kernel FS
  uint32_t gs;         // kernel GS
  uint32_t ldt;        // unused: no LDT
  uint16_t trap;
  uint16_t iomap_base;
} __attribute__((packed)) tss_t;

static gdte_t gdt[GDT_ENTRIES];
static tss_t tss = {};

static void setup_entry(int id,
        uint32_t base, uint32_t limit,
        uint8_t access, uint8_t flags)
{
  gdt[id].base_low    = base & 0xffff;
  gdt[id].base_middle = (base >> 16) & 0xff;
  gdt[id].base_high   = (base >> 24) & 0xff;

  gdt[id].limit       = limit & 0xffff;
  gdt[id].flags       = (limit >> 16) & 0x0f;

  gdt[id].flags      |= (flags & 0xf0);
  gdt[id].access      = access;
}

void gdt_init()
{
  debug("boot32: setting up GDT\n");

  gdtd_t gdt_descr;
  gdt_descr.size = sizeof(gdte_t) * GDT_ENTRIES;
  gdt_descr.addr = gdt;

  setup_entry(0, 0, 0, 0, 0); // null descriptor required by design

  setup_entry(1, 0, 0xFFFFF,
    VALID | ACC_SUP | SYS_NON | CXR, GRAN_4K | OFF_32);  // kcode 0x08
  setup_entry(2, 0, 0xFFFFF,
    VALID | ACC_SUP | SYS_NON | DRW, GRAN_4K | OFF_32);  // kdata 0x10
  setup_entry(3, 0, 0xFFFFF,
    VALID | ACC_USR | SYS_NON | CXR, GRAN_4K | OFF_32);  // ucode 0x18
  setup_entry(4, 0, 0xFFFFF,
    VALID | ACC_USR | SYS_NON | DRW, GRAN_4K | OFF_32);  // udata 0x20

  // task state segment
  setup_entry(5, (uint32_t)&tss, sizeof(tss), 0xe9, 0x00);

  tss.ss0  = 0x10;
  tss.esp0 = 0x00;

  gdt_write(&gdt_descr);
  tss_write();
}
