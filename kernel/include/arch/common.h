#pragma once

#include <arch/definitions.h>

extern void halt_core();

extern void preempt_enable();
extern void preempt_disable();

extern void printdbg(const char *str);
