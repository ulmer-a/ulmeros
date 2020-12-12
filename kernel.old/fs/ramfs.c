#include <fs/vfs.h>
#include <fs/blockdev.h>
#include <list.h>
#include <errno.h>
#include <memory.h>
#include <mutex.h>
#include <arch.h>
#include <kstring.h>

static int ramfs_probe(bd_t* disk)
{
  /* always succeeds because the disk is on RAM */

  (void)disk;
  return -ENOSYS;
}

static int ramfs_mount(bd_t* disk, dir_t* mp)
{
  (void)disk;

  mp->mounted_fs = NULL;
  return SUCCESS;
}

static int ramfs_unmount()
{
  assert(false, "not implemented");
  return -ENOTSUP;
}

static ssize_t ramfs_read(file_t* file, char* buffer,
                    size_t len, size_t offset)
{
  return -ENOSYS;
}

static ssize_t ramfs_write(file_t* file, char* buffer,
                     size_t len, size_t offset)
{
  return -ENOSYS;
}

static const fs_t ramfs = {
  .name = "ramfs",
  .probe = ramfs_probe,
  .mount = ramfs_mount,
  .unmount = ramfs_unmount,
  .read = ramfs_read,
  .write = ramfs_write,
  .mbr_id = 0x00
};

void ramfs_init()
{
  register_fs(&ramfs);
}
