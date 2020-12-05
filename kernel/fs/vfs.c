#include <types.h>
#include <errno.h>
#include <fs/vfs.h>
#include <list.h>

/* built-in filesystems */
extern void ext2fs_load();
extern void ramfs_init();

static list_t* fs_list;
static dir_t root;
static file_t root_file;

void vfs_init()
{
  debug(VFS, "setting up filesystem\n");
  fs_list = list_init();

  root.file = &root_file;
  root.files = list_init();
  root.mounted_fs = NULL;
  root_file.blocks = 0;
  root_file.uid = 0;
  root_file.gid = 0;
  root_file.dir = &root;
  root_file.type = F_DIR;
  root_file.mode.u_r = 1;
  root_file.mode.u_w = 1;
  root_file.mode.u_x = 1;

  /* load known filesystem's drivers */
  ext2fs_load();
  ramfs_init();
}

void register_fs(const fs_t* fs)
{
  debug(VFS, "registering filesystem %s, id=0x%x\n",
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
