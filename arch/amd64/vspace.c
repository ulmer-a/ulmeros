#include <vspace.h>
#include <types.h>
#include <memory.h>
#include <arch/context.h>
#include <mutex.h>

#define IDENT_OFFSET 0xffff800000000000ul

typedef struct
{
  uint64_t present                   :1;
  uint64_t writeable                 :1;
  uint64_t user_access               :1;
  uint64_t write_through             :1;
  uint64_t cache_disabled            :1;
  uint64_t accessed                  :1;
  uint64_t individual                :6;
  uint64_t page_ppn                  :28;
  uint64_t reserved_1                :12;
  uint64_t cow_was_writable          :1;
  uint64_t shared                    :1;
  uint64_t ignored_1                 :9;
  uint64_t noexec                    :1;
} __attribute__((__packed__)) GenericPagingTable;

struct vspace_struct_
{
    size_t pml4_page;
    mutex_t lock;
};

static size_t kernel_pml4_page_;
static GenericPagingTable* kernel_pml4_;

vspace_t vspace_kernel_;

void vspace_init_kernel()
{
  size_t cr3;
  __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));

  kernel_pml4_ = (GenericPagingTable*)(cr3 + IDENT_OFFSET);
  kernel_pml4_page_ = cr3 >> 12;

  vspace_kernel_.pml4_page = kernel_pml4_page_;
  mutex_init(&vspace_kernel_.lock);

  debug(VSPACE, "kernel PML4 at %p\n", kernel_pml4_);
}

void* vspace_get_phys_ptr(void* phys_addr)
{
  return phys_addr + IDENT_OFFSET;
}

void* vspace_get_page_ptr(size_t phys)
{
  return (void*)((phys << 12) + IDENT_OFFSET);
}

vspace_t* vspace_init()
{
  vspace_t* vspace = kmalloc(sizeof(vspace_t));
  vspace->pml4_page = page_alloc(0);

  GenericPagingTable* pml4 = (GenericPagingTable*)
      ((vspace->pml4_page << 12) + IDENT_OFFSET);
  for (int i = 256; i < 512; i++)
    pml4[i] = kernel_pml4_[i];

  return vspace;
}

void vspace_apply(vspace_t *vspace)
{
  size_t phys_pml4 = vspace->pml4_page << 12;
  __asm__ volatile ("mov %0, %%cr3" : : "r"(phys_pml4));
}

vspace_t* vspace_duplicate(vspace_t* original)
{
  vspace_t* vspace = vspace_init();
  assert(false, "vspace_duplicate(): unimplemented");
  return vspace;
}

void vspace_destroy(vspace_t* vspace)
{

}

static void resolve_virt(size_t virt_page, size_t* indices)
{
  indices[3] = (virt_page >> 27) & 0x1ff;
  indices[2] = (virt_page >> 18) & 0x1ff;
  indices[1] = (virt_page >> 9) & 0x1ff;
  indices[0] = (virt_page) & 0x1ff;
}

int vspace_trigger_cow(vspace_t* vspace, size_t virt_page)
{
  (void)vspace; (void)virt_page;
  return true;
}

int vspace_map(vspace_t* vspace, size_t virt_page,
               size_t phys_page, int flags)
{
  vspace_trigger_cow(vspace, virt_page);

  size_t table_indices[4];
  resolve_virt(virt_page, table_indices);

  mutex_lock(&vspace->lock);

  size_t next_table = vspace->pml4_page;
  for (int level = 3; level >= 0; level--)
  {
    GenericPagingTable* entry = (GenericPagingTable*)
        vspace_get_page_ptr(next_table)
        + table_indices[level];

    if (!entry->present)
    {
      // clear entry
      *(uint64_t*)entry = 0;

      // allocate physical page
      entry->page_ppn = page_alloc(0);
      entry->present = 1;

      // set permissions according to flags
      if (level != 0)
      {
        entry->user_access = 1;
        entry->writeable = 1;
      }
      else
      {
        if (flags & PG_USER)    entry->user_access = 1;
        if (flags & PG_WRITE)   entry->writeable   = 1;
        if (flags & PG_NOEXEC)  entry->noexec      = 1;
      }
    }

    next_table = entry->page_ppn;
  }

  mutex_unlock(&vspace->lock);
  return true;
}

int vspace_unmap(vspace_t* vspace, size_t virt_page)
{

  return false;
}

size_t ctx_pf_addr()
{
  size_t addr;
  __asm__ volatile ("mov %%cr2, %0" : "=r"(addr));
  return addr;
}

size_t ctx_pf_error()
{
  return ctx_error(saved_context_);
}

