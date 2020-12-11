#pragma once

#include <proc.h>

typedef struct
{
  int present;
  size_t start_page;
  size_t page_count;
  size_t guard_count;
} ustack_t;

ustack_t* userstack_create(proc_t* proc);
void userstack_delete(ustack_t* stack);
