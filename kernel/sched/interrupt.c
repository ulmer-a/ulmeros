#include <sched/interrupt.h>
#include <arch/common.h>
#include <mm/memory.h>
#include <util/list.h>
#include <debug.h>

static list_t* irq_handlers = NULL;

int irq_ongoing = false;

typedef struct
{
  void (*func)(void*);
  void* drv_data;
} irq_handler_t;

void irq_kernel_init()
{
  list_t* l_irq_handlers = kmalloc(sizeof(list_t) * IRQ_COUNT);
  for (int i = 0; i < IRQ_COUNT; i++)
    list_init(&l_irq_handlers[i]);
  irq_handlers = l_irq_handlers;
}

void irq_subscribe(size_t irq, const char* driver,
                   void (*func)(void*), void *drv)
{
  assert(!irq_ongoing, "irq_subscribe() not available from irq context");
  assert(irq < IRQ_COUNT, "invalid IRQ subscription");

  preempt_disable();
  irq_handler_t* handler = kmalloc(sizeof(irq_handler_t));
  handler->func = func;
  handler->drv_data = drv;
  list_add(&irq_handlers[irq], handler);
  size_t handlers = list_size(&irq_handlers[irq]);
  preempt_enable();

  debug(IRQ, "driver '%s' subscribed for IRQ %zu (%zu handlers installed)\n",
        driver, irq, handlers);
}

void irq_handler(size_t id)
{
  /* the interrupt handler runs in interrupt-context,
   * which means that no locks can be acquired and anyone
   * task could hold any lock. */
  if (irq_handlers == NULL)
    return;

  if (id == 15)
    (void)id;

  for (list_item_t* it = list_it_front(&irq_handlers[id]);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    irq_handler_t* handler = list_it_get(it);
    handler->func(handler->drv_data);
  }
}
