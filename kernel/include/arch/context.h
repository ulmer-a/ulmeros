#pragma once

#include <types.h>

struct arch_context_;
typedef struct arch_context_ arch_context_t;

size_t ctx_irq(arch_context_t* ctx);
size_t ctx_error(arch_context_t* ctx);

/**
 * @brief ctx_pf_error read the page fault address
 * @return the address that caused the page fault
 */
size_t ctx_pf_addr();
size_t ctx_pf_error();

extern arch_context_t* saved_context_;
