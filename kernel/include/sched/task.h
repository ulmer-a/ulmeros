#pragma once

#include <arch/context.h>
#include <mm/vspace.h>

typedef enum
{
  TASK_RUNNING,
  TASK_SLEEPING
} task_state_t;

typedef struct _task_struct
{
  size_t tid;
  task_state_t state;

  context_t* context;
  vspace_t* vspace;

  void* kstack_ptr;
  void* kstack_base;
} task_t;

extern task_t* current_task;

task_t* create_kernel_task(void (*func)(void));
task_t* create_user_task(vspace_t* vspace, void* entry, void* stack_ptr);

int task_schedulable(task_t* task);
