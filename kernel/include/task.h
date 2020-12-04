#pragma once

#include <types.h>
#include <arch/context.h>
#include <vspace.h>

typedef enum
{
  TASK_RUNNING,
  TASK_SLEEPING,
  TASK_IOSLEEP,
  TASK_DELETE
} task_state_t;

typedef struct
{
    size_t tid;
    task_state_t state;
    arch_context_t* context;

    void* kernel_stack;
    size_t kernel_stack_size;

    vspace_t* vspace;
} task_t;

int task_schedulable(task_t* task);

void ktask_init();

void mktask();

/**
 * @brief create_ktask
 * @param func
 * @return
 */
size_t create_ktask(void (*func)());
