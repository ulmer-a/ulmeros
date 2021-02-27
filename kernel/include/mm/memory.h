#pragma once

#include <util/types.h>

#define __init__

void kheap_check_corrupt();
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
void free_page(size_t page);
