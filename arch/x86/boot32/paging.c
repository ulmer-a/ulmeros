#include "boot32.h"

#define PAG_PRESENT BIT(0)
#define PAG_RDWR    BIT(1)

extern void enable_paging(void* pml4);

static void id_map(size_t pml4_ppn, size_t pageCount)
{
  int pml4i = 0;
  int pdpti = 0;
  int pdiri = 0;
  int ptbli = 0;
  uint32_t phys = 0;

  uint64_t* pml4 = (uint64_t*)(pml4_ppn << PAGE_SHIFT);
  uint64_t* pdpt = (uint64_t*)(alloc_page() << PAGE_SHIFT);
  uint64_t* pdir = (uint64_t*)(alloc_page() << PAGE_SHIFT);
  uint64_t* ptbl = (uint64_t*)(alloc_page() << PAGE_SHIFT);

  *((uint64_t*)(pml4 + pml4i)) = (uint64_t)pdpt | PAG_PRESENT | PAG_RDWR;
  *((uint64_t*)(pdpt + pdpti)) = (uint64_t)pdir | PAG_PRESENT | PAG_RDWR;
  *((uint64_t*)(pdir + pdiri)) = (uint64_t)ptbl | PAG_PRESENT | PAG_RDWR;

  while (pageCount--)
  {
    uint64_t phys_addr = (phys++) << PAGE_SHIFT;
    *((uint64_t*)(ptbl + ptbli++)) = (uint64_t)phys_addr | PAG_PRESENT | PAG_RDWR;

    if (ptbli >= 512)
    {
      ptbli = 0;
      ptbl = (uint64_t*)(alloc_page() << PAGE_SHIFT);
      *((uint64_t*)(pdir + (++pdiri))) = (uint64_t)ptbl | PAG_PRESENT | PAG_RDWR;
    }

    if (pdiri >= 511)
    {
      pdiri = 0;
      pdir = (uint64_t*)(alloc_page() << PAGE_SHIFT);
      *((uint64_t*)(pdpt + (++pdpti))) = (uint64_t)pdir | PAG_PRESENT | PAG_RDWR;
    }

    if (pdpti >= 511)
    {
      pdpti = 0;
      pdpt = (uint64_t*)(alloc_page() << PAGE_SHIFT);
      *((uint64_t*)(pml4 + (++pml4i))) = (uint64_t)pdpt | PAG_PRESENT | PAG_RDWR;
    }
  }
}

void paging_init(size_t idmapPages)
{
  alloc_page();

  /* allocate a PML4 level page table and
   * identity-map all RAM that is available. */
  size_t pml4_ppn = alloc_page();
  uint64_t pml4_phys = pml4_ppn << PAGE_SHIFT;
  id_map(pml4_ppn, idmapPages);

  /* next, replicate that mapping at address
   * 0xffff800000000000 (kernel space start) */
  *((uint64_t*)pml4_phys + 256) = *((uint64_t*)pml4_phys);

  /* finally, enable PAE, long mode and paging. */
  debug("enable PAE, long mode, paging... ");
  enable_paging((void*)pml4_phys);
  debug("done\n");
}
