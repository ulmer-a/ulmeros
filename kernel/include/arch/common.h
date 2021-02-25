#pragma once

#include <util/types.h>
#include <arch/definitions.h>

extern void halt_core();

extern void idle();

extern void preempt_enable();
extern void preempt_disable();

extern void printdbg(const char *str);

size_t atomic_add(size_t* mem, ssize_t increment);
