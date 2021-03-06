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
  size_t user_fd;
  fd_t* fd_ptr;
} user_fd_t;

typedef struct _proc_struct
{
  /* PID = process id */
  size_t pid;
  size_t uid;
  size_t gid;

  /* process state */
  proc_state_t state;

  /* a list of tasks (threads) associated
   * with this process. lock with task_list_lock */
  list_t task_list;
  mutex_t task_list_lock;

  /* list of userstack_t objects */
  list_t stack_list;
  mutex_t stack_list_lock;

  size_t fd_counter;
  list_t fd_list;
  mutex_t fd_list_lock;

  /* the process' virtual address space */
  vspace_t* vspace;

  /* heap data */
  size_t heap_brk;
  mutex_t heap_lock;

  dir_t* working_dir;

  loader_t* loader;
} proc_t;

/* start a new process by loading the ELF binary
 * specified with 'filename' and creating a new
 * task and process. */
int proc_start(const char* filename);

int proc_new_fd(proc_t* process, fd_t* fd);
fd_t* proc_get_fd(proc_t* process, int fd);
int proc_dup(proc_t* process, fd_t* fd);
int proc_dup2(proc_t* process, fd_t* fd);
