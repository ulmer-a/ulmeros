#pragma once

#include <util/types.h>

struct arch_context_;
typedef struct arch_context_ context_t;

void preempt_disable();
void preempt_enable();

context_t *context_init(void* kstack_ptr, void* entry_addr, void* stack_ptr,
                        int user, size_t arg0, size_t arg1);

extern context_t* current_context;
