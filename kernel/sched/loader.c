#include <proc.h>
#include <memory.h>
#include <sched/loader.h>
#include <arch.h>
#include <fs/vfs.h>
#include <errno.h>
#include <archdef.h>

loader_t* loader_create(proc_t* process, const char* filename, int* error)
{
  assert(process && filename, "loader_create(): nullpointer checks");

  int err;
  fd_t* fd;
  if ((err = vfs_open(filename, O_READ, &fd)) < 0)
  {
    if (error) *error = -err;
    return NULL;
  }

  bin_t* binary = kmalloc(sizeof(bin_t));
  binary->file = fd;

  if ((err = vfs_read(fd, (char*)&binary->header, sizeof(elf_header64_t))) < 0)
  {
    if (error) *error = -err;
    return NULL;
  }

  assert(ELF_ARCH_BIT_TYPE == 2, "loader: unsupported architecture");

  if (binary->header.magic == 0x464c457f)
  {
    debug(LOADER, "%s: ELF %s bit, %s endian, %s ABI, entry=%p\n",
          filename,
          binary->header.type == 2 ? "64" : "32",
          binary->header.endianness == 2 ? "big" : "little",
          binary->header.os_abi == 0 ? "SystemV" : "unknown",
          binary->header.entry);
  }

  // check if the ELF binary is supported
  if (binary->header.magic != 0x464c457f
      || binary->header.type != ELF_ARCH_BIT_TYPE
      || binary->header.endianness != ELF_ARCH_ENDIAN
      || binary->header.os_abi != 0
      || binary->header.bintype != 2
      || binary->header.isa != ELF_ARCH_ID
      || binary->header.header_size != sizeof(elf_header64_t))
  {
    vfs_close(fd);
    kfree(binary);

    if (error) *error = ENOEXEC;
    return NULL;
  }

  // TODO load PHT

  loader_t* loader = kmalloc(sizeof(loader_t));
  loader->process = process;
  loader->binary = binary;
  return loader;
}

void* loader_get_entry(loader_t* loader)
{
  assert(loader && loader->binary, "loader uninitialized");
  return (void*)loader->binary->header.entry;
}

void loader_duplicate(loader_t* loader, proc_t* proc)
{
  atomic_add(&loader->binary->refs, 1);

  loader_t* new_loader = kmalloc(sizeof(loader_t));
  new_loader->process = proc;
  new_loader->binary = loader->binary;
}

void loader_release(loader_t* loader)
{
  /* if refcount before decrement was 1, we have to
   * delete the binary, too. */
  if (atomic_add(&loader->binary->refs, -1) == 1)
  {
    kfree(loader->binary->program_headers);
    kfree(loader->binary);
  }
  kfree(loader);
}

void loader_map_page(loader_t* loader, size_t virt)
{

}
