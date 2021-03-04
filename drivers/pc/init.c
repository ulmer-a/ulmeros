#include <arch/platform.h>

extern void pc_ata_init();
extern void pc_ps2kbd_init();
extern void pc_vgacon_init();

void platform_init_drivers()
{
  /* initialize drivers based on configuration. */

#ifdef D_ATA
  pc_ata_init();
#endif

#ifdef D_PS2KBD
  pc_ps2kbd_init();
#endif

#ifdef D_VGACON
  pc_vgacon_init();
#endif
}
