#pragma once

#include <types.h>
#include <arch/context.h>

typedef enum
{
  TASK_RUNNING,
  TASK_SLEEPING,
  TASK_DELETE
} task_state_t;

typedef struct
{
    size_t tid;
    task_state_t state;
    arch_context_t* context;

    void* kernel_stack;
    void* ktask_stack;
} task_t;

void ktask_init();

void mktask();

/**
 * @brief create_ktask
 * @param func
 * @return
 */
size_t create_ktask(void* func);
