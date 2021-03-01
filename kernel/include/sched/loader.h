#pragma once

#include <util/types.h>
#include <util/mutex.h>
#include <mm/vspace.h>
#include <fs/vfs.h>

struct _elf64_struct;
typedef struct _elf64_struct elf64_t;

typedef struct
{
  fd_t* file;
  void* entry_addr;
  elf64_t* header;
} loader_t;

loader_t* loader_create(fd_t* file);
