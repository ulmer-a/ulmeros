#pragma once

#include <types.h>

struct arch_context_;
typedef struct arch_context_ arch_context_t;

#define CTX_KERNEL  0
#define CTX_USER    BIT(0)

void arch_yield();

void arch_set_irq_stack(void* stack);

void preempt_disable();
void preempt_enable();

/**
 * @brief ctx_create
 * @param entry
 * @param stack
 * @param flags
 * @return
 */
arch_context_t* ctx_create(void* entry, void *kstack, void* stack, int flags);

/**
 * @brief ctx_set_kernel_stack set the stack that a user space
 * thread uses when it enters kernel space.
 * @param stack_ptr the stack pointer value
 */
void ctx_set_kernel_stack(void* stack_ptr);

void ctx_check_seg(arch_context_t* ctx);

/**
 * @brief ctx_irq get the interrupt number of the interrupt
 * @param ctx  the context
 * @return the irq number
 */
size_t ctx_irq(arch_context_t* ctx);

/**
 * @brief ctx_error get the cpu error of the exception
 * @param ctx the context
 * @return the cpu error
 */
size_t ctx_error(arch_context_t* ctx);

/**
 * @brief ctx_pf_addr read the page fault address
 * @return the address that caused the page fault
 */
size_t ctx_pf_addr();

void ctx_dump(arch_context_t* ctx);

/**
 * @brief ctx_pf_error read the page fault error code
 * @return the error code of the cpu exception
 */
size_t ctx_pf_error();

extern arch_context_t* saved_context;
