#pragma once

#include <arch/context.h>
#include <mm/vspace.h>
#include <sched/proc.h>
#include <sched/userstack.h>

#define KSTACK_SIZE   8192

typedef enum
{
  TASK_RUNNING,
  TASK_SLEEPING,
  TASK_KILLED
} task_state_t;

typedef struct _task_struct
{
  size_t tid;
  task_state_t state;

  context_t* context;
  vspace_t* vspace;

  void* kstack_ptr;
  void* kstack_base;

  int irq_wait;

  size_t user_stack;

  proc_t* process;
} task_t;

extern task_t* current_task;

task_t* create_kernel_task(void (*func)(void));
task_t* create_user_task(vspace_t* vspace, void* entry, userstack_t* stack);

/* kill the current_task */
void task_kill();


void irq_signal(task_t* task);
void irq_wait_until(size_t* cond, size_t value);
int task_schedulable(task_t* task);
