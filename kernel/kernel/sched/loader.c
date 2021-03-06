#include <sched/loader.h>
#include <sched/task.h>
#include <arch/definitions.h>
#include <util/string.h>
#include <mm/memory.h>
#include <fs/vfs.h>
#include <debug.h>
#include <errno.h>

struct _elf64_struct
{
  uint32_t magic;           // should be 0x7f ELF
  uint8_t type;             // 1=32bit, 2=64bit
  uint8_t endianness;       // 1=little, 2=big
  uint8_t header_version;   // header version
  uint8_t os_abi;           // 0=SystemV
  uint64_t reserved1;
  uint16_t bintype;         // 1=reloc, 2=exec, 3=shared, 4=core
  uint16_t isa;             // isa_t
  uint32_t elf_version;     // ELF version
  uint64_t entry;           // program entry point
  uint64_t pht_pos;
  uint64_t sht_pos;
  uint32_t flags;
  uint16_t header_size;
  uint16_t pht_entry_size;
  uint16_t pht_entries;
  uint16_t sht_entry_size;
  uint16_t sht_entries;
  uint16_t sht_index_names;
} __attribute__((packed));

typedef enum
{
  IGNORE    = 0,  /* ignore this segment */
  LOAD      = 1,  /* clear and load */
  DYNAMIC   = 2,  /* requires dynamic linking (currently unimplemented) */
} pht_type_t;

struct _elf64_phte_struct
{
  uint32_t type;
  uint32_t flags;
  uint64_t p_offset;
  uint64_t p_vaddr;
  uint64_t undefined;
  uint64_t p_filesz;
  uint64_t p_memsz;
  uint64_t alignment;
} __attribute__((packed));

static const char* arch_str(uint16_t arch)
{
  switch (arch)
  {
  case 0x00: return "No-Arch";
  case 0x02: return "SPARC";
  case 0x03: return "i386";
  case 0x08: return "MIPS";
  case 0x14: return "PowerPC";
  case 0x28: return "ARM";
  case 0xb7: return "AArch64";
  case 0x32: return "IA64";
  case 0x3e: return "x86_64";
  default: return "Unknown architecture";
  }
}

static int load_pht(loader_t* ldr)
{
  assert(mutex_held(&ldr->lock), "loader lock not held");

  const size_t phte_count = ldr->header->pht_entries;
  const size_t phte_size = ldr->header->pht_entry_size;
  const size_t pht_size = phte_count * phte_size;

  if (phte_size != sizeof(elf64_phte_t))
    return -ENOEXEC;

  debug(LOADER, "loading %zu ELF PHT entries from file\n", phte_count);
  elf64_phte_t* pht = kmalloc(pht_size);
  vfs_seek(ldr->file, ldr->header->pht_pos, SEEK_SET);
  if (vfs_read(ldr->file, pht, pht_size) < pht_size)
  {
    kfree(pht);
    return -ENOEXEC;
  }

  ldr->pht = pht;
  return SUCCESS;
}

static int loader_map_page(loader_t* ldr,
    elf64_phte_t* phte, size_t virt_page, vspace_t* vspace)
{
  assert(mutex_held(&ldr->lock), "loader lock not held");

  /* load the correct ELF file contents into the
   * the newly allocated page, which must have
   * already been cleared to zero.  */
  size_t ppn = alloc_page();
  void* virt_ptr = ppn_to_virt(ppn);

  const size_t f_addr = virt_page << PAGE_SHIFT;

  if (phte->p_vaddr + phte->p_memsz > f_addr)
  {
    /* calculate the amount of bytes to be read from file */
    size_t read_size = PAGE_SIZE;
    if (phte->p_vaddr + phte->p_memsz < f_addr + PAGE_SIZE)
      read_size = phte->p_vaddr + phte->p_memsz - f_addr;

    /* seek to the correct file location */
    const size_t read_offset = phte->p_offset + f_addr - phte->p_vaddr;
    vfs_seek(ldr->file, read_offset, SEEK_SET);

    /* load the memory contents from the ELF binary file */
    assert(read_size <= PAGE_SIZE, "invalid read size calculation");
    if (vfs_read(ldr->file, virt_ptr, read_size) < 0)
    {
      free_page(ppn);
      return -EIO;
    }
  }

  /* set read/write/execute permissions for the page */
  int flags = PG_USER;
  if ((phte->flags & BIT(0)) == 0)  flags |= PG_NOEXEC;
  if (phte->flags & BIT(1))         flags |= PG_WRITE;

  /* map the page */
  vspace_map(vspace, virt_page, ppn, flags);
  debug(LOADER, "loading PPN %zu with code/data @ %p\n",
        ppn, virt_page << PAGE_SHIFT);
  return SUCCESS;
}

static int load_heap_brk(loader_t* ldr)
{
  assert(ldr && ldr->header && ldr->file, "invalid loader_t object");

  mutex_lock(&ldr->lock);
  if (!ldr->pht)
  {
    /* load the program header table from the binary file if
     * it hasn't yet been loaded. */
    int status = load_pht(ldr);
    if (status < 0)
      return status;
  }

  size_t min_heap_break = 0;
  for (size_t i = 0; i < ldr->header->pht_entries; i++)
  {
    elf64_phte_t* phte = &ldr->pht[i];

    /* if the ELF PHT segment type should not be loaded,
     * then don't do anything. */
    if (phte->type != LOAD)
      continue;

    const size_t phte_end_addr = phte->p_vaddr + phte->p_memsz;
    if (phte_end_addr > min_heap_break)
      min_heap_break = ((phte_end_addr >> PAGE_SHIFT) + 1) << PAGE_SHIFT;
  }

  if (min_heap_break == 0)
  {
    assert(false, "invalid heap break");
    ldr->min_heap_break = (size_t)-1;
  }
  else
  {
    ldr->min_heap_break = min_heap_break;
  }
  mutex_unlock(&ldr->lock);
  return SUCCESS;
}

loader_t *loader_create(fd_t *file)
{
  elf64_t* elf_hdr_buf = kmalloc(sizeof(elf64_t));
  vfs_seek(file, 0, SEEK_SET);
  if (vfs_read(file, elf_hdr_buf, sizeof(elf64_t)) < 0)
  {
    kfree(elf_hdr_buf);
    return NULL;
  }

  if (elf_hdr_buf->magic != 0x464c457f)
  {
    debug(LOADER, "ELF magic number not correct\n");
    kfree(elf_hdr_buf);
    return NULL;
  }

  debug(LOADER, "ELF header loaded: %s bit, %s endian %s\n",
        elf_hdr_buf->type == 1 ? "32" : "64",
        elf_hdr_buf->endianness == 1 ? "little" : "big",
        arch_str(elf_hdr_buf->isa));

  if (elf_hdr_buf->isa != ELF_ISA ||
      elf_hdr_buf->endianness != ELF_ENDIANNESS)
  {
    debug(LOADER, "ELF program has incorrect architecture or endianness\n");
    kfree(elf_hdr_buf);
    return NULL;
  }

  loader_t* ldr = kmalloc(sizeof(loader_t));
  mutex_init(&ldr->lock);
  ldr->file = file;
  ldr->entry_addr = (void*)elf_hdr_buf->entry;
  ldr->header = elf_hdr_buf;
  ldr->pht = NULL;
  ldr->refs = 1;
  if (load_heap_brk(ldr) < 0)
    ldr->min_heap_break = (size_t)-1;
  return ldr;
}

int loader_load(loader_t *ldr, size_t virt_page, vspace_t *vspace)
{
  assert(ldr && ldr->header && ldr->file, "invalid loader_t object");

  mutex_lock(&ldr->lock);
  if (!ldr->pht)
  {
    /* load the program header table from the binary file if
     * it hasn't yet been loaded. */
    int status = load_pht(ldr);
    if (status < 0)
      return status;
  }

  const size_t vaddr = virt_page << PAGE_SHIFT;
  for (size_t i = 0; i < ldr->header->pht_entries; i++)
  {
    elf64_phte_t* phte = &ldr->pht[i];

    /* if the ELF PHT segment type should not be loaded,
     * then don't do anything. */
    if (phte->type != LOAD)
      continue;

    /* check whether the fault address lies inbetween the
     * boundaries of the current segment. */
    if (vaddr >= phte->p_vaddr &&
        vaddr < phte->p_vaddr + phte->p_memsz)
    {
      int status = loader_map_page(ldr, phte, virt_page, vspace);
      mutex_unlock(&ldr->lock);
      return status;
    }
  }

  debug(LOADER, "no loadable section refers to address %p\n", vaddr);
  mutex_unlock(&ldr->lock);
  return -ENOENT;
}

void loader_release(loader_t *ldr)
{
  mutex_lock(&ldr->lock);
  int delete_loader = --ldr->refs == 0;
  mutex_unlock(&ldr->lock);

  if (delete_loader)
  {
    /* no more references to the loader are
     * held and we can deallocate it's memory. */
    vfs_close(ldr->file);
    kfree(ldr->header);
    if (ldr->pht)
      kfree(ldr->pht);
    mutex_destroy(&ldr->lock);
    kfree(ldr);
  }
}
