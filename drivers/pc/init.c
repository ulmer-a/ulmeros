#include <arch/platform.h>

extern void pc_ata_init();

void platform_init_drivers()
{
  pc_ata_init();
}
