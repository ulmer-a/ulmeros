#include <types.h>
#include <errno.h>
#include <fs/vfs.h>
#include <list.h>

/* built-in filesystems */
extern void ext2fs_load();

static list_t* fs_list;

void vfs_init()
{
  debug(VFS, "setting up filesystem\n");

  /* load known filesystem's drivers */
  ext2fs_load();

  /* mount the root filesystem */
}

void register_fs(const fs_t* fs)
{
  debug(VFS, "registering filesystem %s, id=%x\n",
        fs->name, fs->mbr_id);
  list_add(fs_list, (void*)fs);
}

ssize_t sys_read(char* buffer, size_t len)
{
  return -ENOSYS;
}

ssize_t sys_write(char* buffer, size_t len)
{
  return -ENOSYS;
}

int sys_open(char* path, int flags, int mode)
{
  return -ENOSYS;
}

int sys_close(int fd)
{
  return -ENOSYS;
}
