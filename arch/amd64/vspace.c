#include <vspace.h>
#include <types.h>
#include <memory.h>

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
  uint64_t execution_disabled        :1;
} __attribute__((__packed__)) GenericPagingTable;

struct vspace_struct_
{
    size_t pml4_page;
};

static size_t kernel_pml4_page_;
static GenericPagingTable* kernel_pml4_;

void vspace_init_kernel()
{
  size_t cr3;
  __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));

  kernel_pml4_ = (GenericPagingTable*)(cr3 + IDENT_OFFSET);
  kernel_pml4_page_ = cr3 >> 12;

  debug(VSPACE, "kernel PML4 at %p\n", kernel_pml4_);
}

vspace_t* vspace_init()
{
  vspace_t* vspace = kmalloc(sizeof(vspace_t));
  vspace->pml4_page = allocPage(0);

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
  assert(false);
  return vspace;
}

void vspace_destroy(vspace_t* vspace)
{

}

int vspace_map(vspace_t* vspace, size_t virt_page,
               size_t phys_page, int user, int shared)
{

  return false;
}

int vspace_unmap(vspace_t* vspace, size_t virt_page)
{

  return false;
}
