#pragma once

#include <types.h>

#define IRQ_ATA_PRIM    14
#define IRQ_ATA_SEC     15

void _init irq_init();
void _init arch_irq_init();

/**
 * @brief irq_register register an interrupt
 * @param irq the irq number to subscribe
 * @param handler the interrupt handler routine
 * @param drv custom driver data that is passed to the handler
 * @return int the handler descriptor
 */
int irq_register(size_t irq, void (*handler)(void*), void* drv);

/**
 * @brief irq_unregister unregister an interrupt handler
 * @param descriptor the descriptor returned by irq_register()
 */
void irq_unregister(int descriptor);

uint64_t timer_ticks();
