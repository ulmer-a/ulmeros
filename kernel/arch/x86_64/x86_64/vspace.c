/*
 * UlmerOS x86_64 Virtual memory implementation
 * Copyright (C) 2021 Alexander Ulmer
 */

#include <mm/vspace.h>
#include <mm/memory.h>
#include <util/string.h>
#include <debug.h>

#define IDENT_OFFSET 0xffff800000000000ul

vspace_t _vspace_kernel;

static void tlb_invalidate(size_t virt)
{
  virt <<= PAGE_SHIFT;
  __asm__ volatile ("invlpg (%0)" : : "b"(virt) : "memory");
}

/*
 * the generic page table entry structure
 * defines the fields that entries on all
 * levels in the page table hierarchy have
 * in common. other fields should be set to
 * zero (either because reserved or must be
 * zero)
 */
typedef struct
{
  uint64_t present        : 1;
  uint64_t write          : 1;
  uint64_t user           : 1;
  uint64_t write_through  : 1;
  uint64_t cache_disable  : 1;
  uint64_t accessed       : 1;
  uint64_t dirty          : 1;
  uint64_t zero           : 2;
  uint64_t available      : 3;
  uint64_t ppn            : 40;
  uint64_t available2     : 11;
  uint64_t no_exec        : 1;
} gpte_t;

typedef struct
{
  /* physical page numbers of the respective
   * page translation level. for example, PML4
   * table is located at physical page pml4_ppn. */
  size_t pml4_ppn, pdpt_ppn,
    pdir_ppn, ptbl_ppn, page_ppn;


  /* indices into the respective table. */
  size_t pml4i, pdpti, pdiri, ptbli;

  /* pointers to the respective table's entry. it
   * can be directly used to modify the respective
   * page table. */
  gpte_t *pml4e, *pdpte, *pdire, *ptble;

  /* a pointer to the content of the page mapped
   * at that virtual address. */
  void* page;
} vaddr_t;

void vspace_setup(size_t pml4_ppn)
{
  assert(sizeof(gpte_t) == 8, "gpte_t is not of size 64bit");
  debug(INIT, "PML4 uses page frame %zu\n", pml4_ppn);
  _vspace_kernel.pml4_ppn = pml4_ppn;

  /* clear the lower identity mapping, so we can
   * detect nullpointer dereferences and similar errors. */
  memset(ppn_to_virt(_vspace_kernel.pml4_ppn), 0, PAGE_SIZE/2);
  vspace_apply(VSPACE_KERNEL);
}

static void vspace_update_kernel_mapping(vspace_t* vspace)
{
  assert(vspace != VSPACE_KERNEL,
         "update_kernel_mapping(): VSPACE_KERNEL given");

  /* to refresh the kernel mappings in a given address
   * space by copying the upper half of the PML4 of the
   * kernel into the target address space. */
  memcpy(ppn_to_virt(vspace->pml4_ppn) + PAGE_SIZE/2,
         ppn_to_virt(_vspace_kernel.pml4_ppn) + PAGE_SIZE/2,
         PAGE_SIZE/2);
}

vspace_t* vspace_create()
{
  vspace_t* vspace = kmalloc(sizeof(vspace_t));
  vspace->pml4_ppn = alloc_page();
  vspace_update_kernel_mapping(vspace);
  return vspace;
}

void* phys_to_virt(void* phys_addr)
{
  return phys_addr + IDENT_OFFSET;
}

void* ppn_to_virt(size_t ppn)
{
  return phys_to_virt((void*)(ppn << PAGE_SHIFT));
}

static void resolve_mapping(vspace_t* vspace, size_t virt, vaddr_t* vaddr)
{
  vaddr->ptbli = virt & 0x1ff;
  vaddr->pdiri = (virt >> 9) & 0x1ff;
  vaddr->pdpti = (virt >> 18) & 0x1ff;
  vaddr->pml4i = (virt >> 27) & 0x1ff;

  vaddr->pml4_ppn = vspace->pml4_ppn;
  vaddr->pml4e = (gpte_t*)ppn_to_virt(vaddr->pml4_ppn) + vaddr->pml4i;

  if (vaddr->pml4e->present)
  {
    vaddr->pdpt_ppn = vaddr->pml4e->ppn;
    vaddr->pdpte = (gpte_t*)ppn_to_virt(vaddr->pdpt_ppn) + vaddr->pdpti;

    if (vaddr->pdpte->present)
    {
      vaddr->pdir_ppn = vaddr->pdpte->ppn;
      vaddr->pdire = (gpte_t*)ppn_to_virt(vaddr->pdir_ppn) + vaddr->pdiri;

      if (vaddr->pdire->present)
      {
        vaddr->ptbl_ppn = vaddr->pdire->ppn;
        vaddr->ptble = (gpte_t*)ppn_to_virt(vaddr->ptbl_ppn) + vaddr->ptbli;

        if (vaddr->ptble->present)
        {
          vaddr->page_ppn = vaddr->ptble->ppn;
          vaddr->page = (gpte_t*)ppn_to_virt(vaddr->page_ppn);
        }
        else
        {
          vaddr->page_ppn = 0;
          vaddr->page = NULL;
        }
      }
      else
      {
        vaddr->ptbl_ppn = 0;
        vaddr->page_ppn = 0;
        vaddr->ptble = NULL;
        vaddr->page = NULL;
      }
    }
    else
    {
      vaddr->pdir_ppn = 0;
      vaddr->ptbl_ppn = 0;
      vaddr->page_ppn = 0;
      vaddr->pdire = NULL;
      vaddr->ptble = NULL;
      vaddr->page = NULL;
    }
  }
  else
  {
    vaddr->pdpt_ppn = 0;
    vaddr->pdir_ppn = 0;
    vaddr->ptbl_ppn = 0;
    vaddr->page_ppn = 0;
    vaddr->pdpte = NULL;
    vaddr->pdire = NULL;
    vaddr->ptble = NULL;
    vaddr->page = NULL;
  }
}

void vspace_map(vspace_t *vspace, size_t virt, size_t phys, int flags)
{
  vaddr_t vaddr;
  resolve_mapping(vspace, virt, &vaddr);

  if (!vaddr.pml4e->present)
  {
    vaddr.pml4e->present = 1;
    vaddr.pml4e->write = 1;
    vaddr.pml4e->user = 1;
    vaddr.pml4e->ppn = alloc_page();
    resolve_mapping(vspace, virt, &vaddr);
  }

  if (!vaddr.pdpte->present)
  {
    vaddr.pdpte->present = 1;
    vaddr.pdpte->write = 1;
    vaddr.pdpte->user = 1;
    vaddr.pdpte->ppn = alloc_page();
    resolve_mapping(vspace, virt, &vaddr);
  }

  if (!vaddr.pdire->present)
  {
    vaddr.pdire->present = 1;
    vaddr.pdire->write = 1;
    vaddr.pdire->user = 1;
    vaddr.pdire->ppn = alloc_page();
    resolve_mapping(vspace, virt, &vaddr);
  }

  if (!vaddr.ptble->present)
  {
    vaddr.ptble->present = 1;
    vaddr.ptble->no_exec = (flags & PG_NOEXEC) ? 1 : 0;
    vaddr.ptble->write = (flags & PG_WRITE) ? 1 : 0;
    vaddr.ptble->user = (flags & PG_USER) ? 1 : 0;
    vaddr.ptble->ppn = phys;
  }

  debug(VSPACE, "mapped PPN %zu @ %p\n", phys, virt << PAGE_SHIFT);
  tlb_invalidate(virt);
}

void vspace_unmap(vspace_t *vspace, size_t virt)
{
  vaddr_t vaddr;
  resolve_mapping(vspace, virt, &vaddr);

  if (vaddr.page)
  {
    *(uint64_t*)vaddr.ptble = 0;
    free_page(vaddr.page_ppn);
  }
  debug(VSPACE, "unmapped PPN %zu @ %p\n", vaddr.page_ppn, virt << PAGE_SHIFT);
  tlb_invalidate(virt);
}

void vspace_apply(vspace_t *vspace)
{
  void* phys_addr = (void*)(vspace->pml4_ppn << PAGE_SHIFT);
  __asm __volatile__ ("mov %0, %%cr3;" : : "r"(phys_addr));
}

void* virt_to_phys(vspace_t* vspace, void *virt_addr)
{
  size_t ppn = virt_to_ppn(vspace, virt_addr);
  return (void*)(ppn << PAGE_SHIFT);
}

size_t virt_to_ppn(vspace_t* vspace, void *virt_addr)
{
  vaddr_t vaddr;
  size_t virt = (size_t)virt_addr >> PAGE_SHIFT;
  resolve_mapping(vspace, virt, &vaddr);
  return vaddr.page_ppn;
}
