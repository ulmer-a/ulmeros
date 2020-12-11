#pragma once

#include <types.h>
#include <mutex.h>
#include <fs/vfs.h>

struct proc_struct;
typedef struct proc_struct proc_t;

typedef struct
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
} __attribute__((packed)) elf_header64_t;

typedef struct
{

} elf_pht_t;

typedef struct
{

} pht_entry_flags_t;

typedef struct
{
  fd_t* file;
  elf_header64_t header;
  size_t program_header_count;
  elf_pht_t* program_headers;
  size_t refs;
} bin_t;

typedef struct
{
  proc_t* process;
  bin_t* binary;
} loader_t;

loader_t* loader_create(proc_t *process, const char* filename, int *error);
void* loader_get_entry(loader_t* loader);
void loader_duplicate(loader_t* loader, proc_t *proc);
void loader_release(loader_t* loader);

void loader_map_page(loader_t* loader, size_t virt);
