#pragma once

#include <list.h>
#include <mutex.h>
#include <vspace.h>
#include <sched/loader.h>

typedef struct proc_struct
{
  size_t pid;

  list_t* threads;
  mutex_t threads_lock;

  list_t* stacks;
  mutex_t stacks_lock;

  vspace_t* addr_space;

  loader_t* loader;

} proc_t;

size_t proc_create_initial(const char *filename);
