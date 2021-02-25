#include <util/types.h>
#include <sched/interrupt.h>

/* set to 1 during interrupt handling by
 * architecture specific code */
int irq_ongoing;

void irq_handler(size_t id)
{
  (void)id;
}
