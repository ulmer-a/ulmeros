#pragma once

#include <util/types.h>

#define __init__

void _kheap_check_corrupt(const char* func, unsigned line);
#define kheap_check_corrupt() _kheap_check_corrupt(__func__, __LINE__);
void kheap_print();

#ifdef DEBUG
void* _kmalloc(size_t size, unsigned line, const char *function);
#define kmalloc(n) _kmalloc((n), __LINE__, __func__)
#else
void* _kmalloc(size_t size);
#define kmalloc(n) _kmalloc((n))
#endif

void kfree(void* ptr);

size_t alloc_page();
void* alloc_dma_region();
void free_page(size_t page);
