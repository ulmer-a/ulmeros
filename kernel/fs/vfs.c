#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <fs/blockdev.h>
#include <util/string.h>
#include <arch/common.h>
#include <debug.h>
#include <errno.h>

dir_t _vfs_root;

static list_t fs_list;
static mutex_t fs_list_lock;

extern void ext2fs_init();

static void* probe_fs(fd_t* disk, fs_t** fs_)
{
  mutex_lock(&fs_list_lock);
  for (list_item_t* it = list_it_front(&fs_list);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    fs_t* fs = list_it_get(it);
    void* data = fs->probe(disk);
    if (data)
    {
      mutex_unlock(&fs_list_lock);
      *fs_ = fs;
      return data;
    }
  }
  mutex_unlock(&fs_list_lock);
  return NULL;
}

void vfs_init(const char* rootfs)
{
  debug(VFS, "initializing virtual filesystem\n");
  list_init(&fs_list);
  mutex_init(&fs_list_lock);

  /* initialize filesystem drivers */
  ext2fs_init();

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

  fs_t* fs;
  void* fsdata = probe_fs(fd, &fs);
  if (fsdata == NULL)
  {
    debug(VFS, "error: unknown file system\n");
    assert(false, "Cannot mount root file system\n");
    return;
  }

  // TODO: mount root filesystem
}

void fs_register(fs_t *fs)
{
  assert(fs, "invalid fs");
  mutex_lock(&fs_list_lock);
  list_add(&fs_list, fs);
  mutex_unlock(&fs_list_lock);
}

ssize_t vfs_read(fd_t *fd, void *buffer, uint64_t length)
{
  if (fd->f_ops.read == NULL)
    return -ENOTSUP;
  return fd->f_ops.read(fd->fs_data, buffer, length, fd->fpos);
}

uint64_t vfs_seek(fd_t *fd, uint64_t offset, int whence)
{
  switch (whence)
  {
  case SEEK_SET:
    fd->fpos = offset;
    break;

  case SEEK_CUR:
    fd->fpos += offset;
    break;

  case SEEK_END:
    fd->fpos = fd->file->length + offset;
    break;
  }
  return offset;
}

fd_t *vfs_dup(fd_t *fd)
{
  return fd;
}
