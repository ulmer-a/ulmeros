#pragma once

#include <types.h>

#define IRQ_ATA_PRIM    14
#define IRQ_ATA_SEC     15

void _init irq_init();
void _init arch_irq_init();

void irq_install_handler(unsigned irq, int (*handler)());
void irq_uninstall_handler(unsigned irq);

uint64_t timer_ticks();
