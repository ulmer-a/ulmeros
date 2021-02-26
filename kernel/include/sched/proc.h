#pragma once

#include <util/types.h>
#include <util/mutex.h>
#include <util/list.h>
#include <mm/vspace.h>

typedef struct
{
  /* PID = process id */
  size_t pid;

  /* a list of tasks (threads) associated
   * with this process. lock with task_list_lock */
  list_t task_list;
  mutex_t task_list_lock;

  list_t stack_list;

  /* the process' virtual address space */
  vspace_t* vspace;

} proc_t;

/* start a new process by loading the ELF binary
 * specified with 'filename' and creating a new
 * task and process. */
void proc_start(const char* filename);
