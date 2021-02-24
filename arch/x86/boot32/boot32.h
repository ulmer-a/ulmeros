#pragma once

#include <util/types.h>
#include "multiboot.h"

#define EARLY_HEAP_START  0x02000000
#define KERNEL_LOAD_ADDR  0x01000000

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

#include "../include/x86/bootinfo.h"

extern void halt_core();
extern void gdt_init();
extern void* gdt_long_init();
extern void print_heap();
extern size_t paging_init(size_t idmapPages);
extern uint64_t create_mmap(multiboot_mmape_t *mmap, size_t length);
extern void create_pagemap(uint64_t *addr, uint64_t *total_pages_ptr);

extern size_t alloc_page();

extern void gdt_write(void* addr);
extern void tss_write();
extern void jmp_longmode(void* arg);

#ifdef DEBUG
void _assert(const char* function, const char* file, int line, const char *msg);
#define assert(x, msg) if (!(x)) { _assert(__func__, __FILE__, __LINE__, msg); }
#else
#define assert(x, msg)
#endif

extern void debug(const char* fmt, ...);

void *kmalloc(size_t size);
void kfree(void* ptr);

extern char _bss_start;
extern char _bss_end;

