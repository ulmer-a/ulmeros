#pragma once

#include <util/types.h>


void setup_page_bitmap(void* bitmap_addr, size_t size);

void vspace_setup(size_t pml4_ppn);
void kheap_init();

