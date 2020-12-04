#include "boot32.h"

#define PAG_PRESENT BIT(0)
#define PAG_RDWR    BIT(1)

static void enablePae()
{
    __asm__ volatile("mov %cr4, %eax;"
                     "or $(1 << 5), %eax;"
                     "mov %eax, %cr4;");
}

static void loadAddressSpace(void *pt_ptr)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(pt_ptr));
}

void idMapPages(uint64_t* pml4, unsigned long count)
{
  int pml4i = 0;
  int pdpti = 0;
  int pdiri = 0;
  int ptbli = 1;
  uint32_t phys = 1;

  uint64_t* pdpt = getEmptyPage();
  uint64_t* pdir = getEmptyPage();
  uint64_t* ptbl = getEmptyPage();

  *((uint32_t*)(pml4 + pml4i)) = (uint32_t)pdpt | PAG_PRESENT | PAG_RDWR;
  *((uint32_t*)(pdpt + pdpti)) = (uint32_t)pdir | PAG_PRESENT | PAG_RDWR;
  *((uint32_t*)(pdir + pdiri)) = (uint32_t)ptbl | PAG_PRESENT | PAG_RDWR;

  while (count--)
  {
    unsigned long phys_addr = (phys++) * PAGE_SIZE;
    *((uint32_t*)(ptbl + ptbli++)) = (uint32_t)phys_addr | PAG_PRESENT | PAG_RDWR;

    if (ptbli >= 512)
    {
      ptbli = 0;
      ptbl = getEmptyPage();
      *((uint32_t*)(pdir + (++pdiri))) = (uint32_t)ptbl | PAG_PRESENT | PAG_RDWR;
    }

    if (pdiri >= 511)
    {
      pdiri = 0;
      pdir = getEmptyPage();
      *((uint32_t*)(pdpt + (++pdpti))) = (uint32_t)pdir | PAG_PRESENT | PAG_RDWR;
    }

    if (pdpti >= 511)
    {
      pdpti = 0;
      pdpt = getEmptyPage();
      *((uint32_t*)(pml4 + (++pml4i))) = (uint32_t)pdpt | PAG_PRESENT | PAG_RDWR;
    }
  }
}

void* setupPaging()
{
  uint64_t* pml4 = getEmptyPage();
  unsigned long physicalPages = ramSize / PAGE_SIZE;
  idMapPages(pml4, physicalPages);
  *((uint32_t*)(pml4 + 256)) = *((uint32_t*)(pml4));

  loadAddressSpace(pml4);
  enablePae();
  return pml4;
}
