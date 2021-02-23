#pragma once

#include <util/types.h>

extern uint64_t mm_ram_size;

void page_init();
size_t get_page_bmp_size();
void page_bmp_copy(void* location);

int add_region(uint64_t addr, uint64_t size, int available, ssize_t replaceId);
