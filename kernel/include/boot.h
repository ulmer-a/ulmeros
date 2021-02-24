#pragma once

#include <util/bitmap.h>

typedef struct
{
  bitmap_t* free_pages;
  void* bootloader_load_addr;
} bootinfo_t;
