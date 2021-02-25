#include <util/types.h>
#include <mm/memory.h>
#include <debug.h>

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

struct gdtd_struct // GDT descriptor for CPU
{
  uint16_t size;
  struct gdte_struct *addr;
} __attribute__((packed));

struct gdte_struct // GDT entry
{
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t flags0;
  uint8_t flags1_limit;
  uint8_t base_high;
} __attribute__((packed));

struct gdt_page_struct
{
  struct gdtd_struct descriptor;
  struct gdte_struct gdt[GDT_ENTRIES];
} __attribute__((packed));

static void setup_entry(struct gdt_page_struct* gdt, int id,
        uint32_t base, uint32_t limit, uint32_t flags)
{
  gdt->gdt[id].base_low    = base & 0xffff;
  gdt->gdt[id].base_middle = (base >> 16) & 0xff;
  gdt->gdt[id].base_high   = (base >> 24) & 0xff;

  gdt->gdt[id].limit = limit & 0xffff;
  gdt->gdt[id].flags1_limit = (limit >> 16) & 0b1111;

  *((uint32_t*)&(gdt->gdt[id].base_middle)) |= flags;
}

void reload_gdt()
{
  struct gdt_page_struct* gdt = kmalloc(sizeof(struct gdt_page_struct));

  gdt->descriptor.size = sizeof(struct gdte_struct) * GDT_ENTRIES;
  gdt->descriptor.addr = gdt->gdt;

  setup_entry(gdt, 0, 0, 0xf0000, GRAN_4K); // null descriptor required by design

  setup_entry(gdt, 1, 0, 0xf0000, PRESENT | GRAN_4K | LONGMODE | ACC_EXC_RD | CODESEG);  // kcode
  setup_entry(gdt, 2, 0, 0xf0000, PRESENT | GRAN_4K | LONGMODE | ACC_EXC_RD | DATASEG);  // kdata
  setup_entry(gdt, 3, 0, 0xf0000, PRESENT | GRAN_4K | LONGMODE | ACC_EXC_RD | CODESEG | DPL_USER);  // ucode
  setup_entry(gdt, 4, 0, 0xf0000, PRESENT | GRAN_4K | LONGMODE | ACC_EXC_RD | DATASEG | DPL_USER);  // udata

  debug(INIT, "reloading GDT... ");
  __asm__ volatile("lgdt %0" : : "g"(gdt->descriptor));
  debug(INIT, "done\n");

  /* verify the GDT by loading a segment selector*/
  __asm__ volatile(
    "mov $0x10, %ax;"
    "mov %ax, %ds;"
    "mov %ax, %ss;"
  );
}
