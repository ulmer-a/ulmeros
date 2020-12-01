#pragma once

#include <types.h>

size_t xchg(size_t val, size_t* mem);
size_t atomic_add(size_t* mem, ssize_t increment);

void timer_reset(uint16_t freqHz);

void arch_idle();
