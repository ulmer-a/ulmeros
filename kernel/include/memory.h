#pragma once

#include <types.h>

void* kmalloc(size_t size);
void kfree(void* ptr);

size_t allocPage(int flags);
void releasePage(size_t page);
