#pragma once

#include <util/types.h>
#include <arch/platform.h>

void irq_kernel_init();
void irq_subscribe(size_t irq, const char *driver,
                   void (*func)(void *), void* drv);

extern int irq_ongoing;
