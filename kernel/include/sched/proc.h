#pragma once

#include <util/types.h>
#include <sched/mutex.h>
#include <util/list.h>
#include <mm/vspace.h>
#include <fs/vfs.h>
#include <sched/loader.h>

typedef enum
{
  PROC_RUNNING,
  PROC_KILLED
} proc_state_t;

typedef struct
{
  /* PID = process id */
  size_t pid;

  /* process state */
  proc_state_t state;

  /* a list of tasks (threads) associated
   * with this process. lock with task_list_lock */
  list_t task_list;
  mutex_t task_list_lock;

  list_t stack_list;
  mutex_t stack_list_lock;

  /* the process' virtual address space */
  vspace_t* vspace;

  dir_t* working_dir;

  loader_t* loader;
} proc_t;

/* start a new process by loading the ELF binary
 * specified with 'filename' and creating a new
 * task and process. */
void proc_start(const char* filename);

int proc_new_fd(proc_t* process, fd_t* fd);
fd_t* proc_get_fd(proc_t* process, int fd);
int proc_dup(proc_t* process, fd_t* fd);
int proc_dup2(proc_t* process, fd_t* fd);
