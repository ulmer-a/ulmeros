#pragma once

#include <util/types.h>
#include <arch/definitions.h>

/* the generic kernel main function. it is
 * called by architecture specific code once
 * basic initialization has completed. */
extern void kmain(const char* cmdline, void* initrd, size_t initrd_size);

/* after kmain() executed and the kernel is
 * running in multitasking mode, we can deallocate
 * the initialization stack. */
extern void delete_init_stack();

/* stop CPU core forever */
extern void halt_core();

/* halt CPU until interrupt/exception fires */
extern void idle();

/* enable/disable interrupts */
extern void preempt_enable();
extern void preempt_disable();

/* the platform's native debug output
 * (for example the 0xE9 port on QEMU) */
extern void printdbg(const char *str);

/* set the stack pointer that will be used
 * when returning from user mode. */
void set_kernel_sp(uint64_t sp);

size_t atomic_add(size_t* mem, ssize_t increment);
