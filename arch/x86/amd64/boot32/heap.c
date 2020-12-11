#include "boot32.h"

#define PAGE_SIZE 4096

extern char _bss_start;
extern char _bss_end;

void clearBss()
{
    unsigned long bss_length = (unsigned long)&_bss_end - (unsigned long)&_bss_start;
    unsigned char *bss_ptr = (unsigned char *)&_bss_start;

    while (bss_length--)
        *bss_ptr++ = 0;
}

void memcpy(char *dest, const char *src, unsigned long n)
{
    while (n--)
        *dest++ = *src++;
}

void memset(void* mem, char val, unsigned long size)
{
  char *ptr = mem;
  while (size--)
    *ptr++ = val;
}

static unsigned long start_page = 8192;
static unsigned long pages = 0;

void* getEmptyPage()
{
  void* mem = (void*)((start_page + pages++) * PAGE_SIZE);
  memset(mem, 0, PAGE_SIZE);
  return mem;
}

uint32_t getHeapStart()
{
  return start_page;
}

uint32_t getHeapSize()
{
  return pages;
}
