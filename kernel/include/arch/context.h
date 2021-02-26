#pragma once

#include <util/types.h>

struct arch_context_;
typedef struct arch_context_ context_t;

void preempt_disable();
void preempt_enable();

/* set the program counter of the given context */
void context_set_pc(context_t* ctx, size_t pc);

/* set the return value of the given context */
void context_set_ret(context_t* ctx, size_t ret);

context_t *context_init(void* kstack_ptr, void* entry_addr, void* stack_ptr,
                        int user, size_t arg0, size_t arg1);

extern context_t* current_context;
