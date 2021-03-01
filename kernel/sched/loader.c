#include <sched/loader.h>
#include <mm/memory.h>
#include <arch/definitions.h>
#include <debug.h>

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

loader_t *loader_create(fd_t *file)
{
  elf64_t* elf_hdr_buf = kmalloc(sizeof(elf64_t));
  vfs_seek(file, 0, SEEK_SET);
  if (vfs_read(file, elf_hdr_buf, sizeof(elf64_t)) < 0)
    return NULL;

  if (elf_hdr_buf->magic != 0x464c457f)
  {
    debug(LOADER, "ELF magic number not correct\n");
    return NULL;
  }

  debug(LOADER, "ELF header loaded: %s bit, %s endian %s\n",
        elf_hdr_buf->type == 1 ? "32" : "64",
        elf_hdr_buf->endianness == 1 ? "little" : "big",
        arch_str(elf_hdr_buf->isa));

  loader_t* ldr = kmalloc(sizeof(loader_t));
  ldr->file = file;
  ldr->entry_addr = (void*)elf_hdr_buf->entry;
  ldr->header = elf_hdr_buf;
  return ldr;
}
