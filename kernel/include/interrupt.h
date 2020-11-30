#pragma once

#include <types.h>

void _init irq_init();
void _init arch_irq_init();

void irq_install_handler(unsigned irq, void (*handler)());
void irq_uninstall_handler(unsigned irq);

uint64_t timer_ticks();
