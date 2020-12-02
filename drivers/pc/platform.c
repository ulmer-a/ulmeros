#include <arch/platform.h>

extern void ata_init();

void platform_init_drivers()
{
  ata_init();
}
