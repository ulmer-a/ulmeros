#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <debug.h>

dir_t _vfs_root;

void vfs_init(const char* rootfs)
{
  debug(VFS, "initializing virtual filesystem\n");
  debug(VFS, "mounting '%s' at /\n", rootfs);

  /* mount the root filesystem. */
}
