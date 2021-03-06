#pragma once

#include <util/types.h>
#include <sched/mutex.h>
#include <mm/vspace.h>
#include <fs/vfs.h>

/* the ELF file header */
struct _elf64_struct;
typedef struct _elf64_struct elf64_t;

/* a single ELF program header table entry */
struct _elf64_phte_struct;
typedef struct _elf64_phte_struct elf64_phte_t;


typedef struct
{
  fd_t* file;
  void* entry_addr;
  elf64_t* header;
  elf64_phte_t* pht;
  mutex_t lock;
  size_t refs;
  size_t min_heap_break;
} loader_t;

loader_t* loader_create(fd_t* file);
void loader_release(loader_t* ldr);
int loader_load(loader_t* ldr, size_t virt_page, vspace_t* vspace);
