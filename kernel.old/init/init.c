#include <fs/vfs.h>
#include <arch/platform.h>
#include <bus/pci.h>
#include <kstring.h>
#include <config.h>
#include <proc.h>

void init_userspace();

void sysinit_task()
{
  debug(KMAIN, "Hello from the kernel initialization task!\n");
  /* initialize blockdevice manager */
  bd_init();

  /* perform a PCI scan, if present */
  pci_init();

  /* load the platform drivers */
  platform_init_drivers();

  /* initialize the virtual file system. this will load
   * all built in file system drivers. */
  vfs_init();

  /* mount the root filesystem */
  size_t root_major = 1, root_minor = 0;
  int err;
  if ((err = vfs_mount(VFS_ROOT, root_major, root_minor)) < 0)
  {
    debug(KMAIN, "root fs mount failed: %s\n", strerror(-err));
  }

  init_userspace();

  debug(KMAIN, "done\n");
}

void init_userspace()
{
  const char* init_program = USERSPACE_START;
  debug(KMAIN, "starting userspace init (%s)\n", init_program);

  /* spawn the first userspace program by loading
   * the binary from the path passed as an argument. */
  proc_create_initial("/bin/init.bin");
}
