#pragma once

typedef unsigned long long target_word_t;

struct boot_info_
{
  target_word_t ramPages;
  target_word_t heapStartPage;
  target_word_t heapPageCount;
  target_word_t gdtPage;
  target_word_t kernelStartPage;
  target_word_t kernelPageCount;
  target_word_t pml4Page;
  target_word_t mmap;
  target_word_t mmap_length;
};

typedef struct boot_info_ boot_info_t;
