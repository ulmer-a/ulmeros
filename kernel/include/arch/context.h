#pragma once

#include <types.h>

struct arch_context_;
typedef struct arch_context_ arch_context_t;

#define CTX_KERNEL  0
#define CTX_USER    BIT(0)

arch_context_t* ctx_create(void* entry, void* stack, int flags);

/**
 * @brief ctx_irq get the interrupt number of the interrupt
 * @param ctx the irq number
 * @return
 */
size_t ctx_irq(arch_context_t* ctx);

/**
 * @brief ctx_error get the cpu error of the exception
 * @param ctx the cpu error
 * @return
 */
size_t ctx_error(arch_context_t* ctx);

/**
 * @brief ctx_pf_addr read the page fault address
 * @return the address that caused the page fault
 */
size_t ctx_pf_addr();

/**
 * @brief ctx_pf_error read the page fault error code
 * @return the error code of the cpu exception
 */
size_t ctx_pf_error();

extern arch_context_t* saved_context_;
