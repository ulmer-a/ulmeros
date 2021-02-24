#pragma once

#include <util/types.h>

struct arch_context_;
typedef struct arch_context_ context_t;

void preempt_disable();
void preempt_enable();

context_t* ctx_create(void* entry, void *kstack, void* stack, int flags);

extern context_t* current_context;
