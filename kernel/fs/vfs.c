#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <fs/blockdev.h>
#include <util/string.h>
#include <debug.h>
#include <errno.h>

dir_t _vfs_root;

static fs_t* probe_fs(fd_t* disk)
{
  return NULL;
}

void vfs_init(const char* rootfs)
{
  debug(VFS, "initializing virtual filesystem\n");
  debug(VFS, "trying to mount '%s' at /\n", rootfs);

  size_t major, minor;
  if (!bd_get_by_name(rootfs, &major, &minor))
  {
    debug(VFS, "error: cannot mount rootfs: no such device\n");
    assert(false, "Cannot mount root file system\n");
    return;
  }

  fd_t* fd;
  int error;
  if ((error = bd_open(&fd, major, minor)) < 0)
  {
    debug(VFS, "error: cannot open device: %s\n", strerror(-error));
    assert(false, "Cannot mount root file system\n");
    return;
  }

  fs_t* fs = probe_fs(fd);
  if (fs == NULL)
  {
    debug(VFS, "error: unknown file system\n");
    assert(false, "Cannot mount root file system\n");
    return;
  }

  // TODO: mount root filesystem
}
