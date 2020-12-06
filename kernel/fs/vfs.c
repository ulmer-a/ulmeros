#include <types.h>
#include <errno.h>
#include <fs/vfs.h>
#include <list.h>
#include <mutex.h>

/* built-in filesystems */
extern void ext2fs_load();
extern void ramfs_init();

static list_t* fs_list;
static mutex_t fs_list_lock = MUTEX_INITIALIZER;
dir_t vfs_root_node;
static file_t root_file;

void vfs_init()
{
  debug(VFS, "setting up filesystem\n");
  fs_list = list_init();

  vfs_root_node.file = &root_file;
  vfs_root_node.files = list_init();
  vfs_root_node.mounted_fs = NULL;
  root_file.blocks = 0;
  root_file.uid = 0;
  root_file.gid = 0;
  root_file.dir = &vfs_root_node;
  root_file.type = F_DIR;
  root_file.mode.u_r = 1;
  root_file.mode.u_w = 1;
  root_file.mode.u_x = 1;
  root_file.parent = &vfs_root_node;

  /* load known filesystem's drivers */
  ext2fs_load();
  ramfs_init();
}

void register_fs(const fs_t* fs)
{
  debug(VFS, "registering filesystem %s, id=0x%x\n",
        fs->name, fs->mbr_id);
  mutex_lock(&fs_list_lock);
  list_add(fs_list, (void*)fs);
  mutex_unlock(&fs_list_lock);
}

int vfs_mount(dir_t* mountpoint, size_t major, size_t minor)
{
  bd_t* bd = bd_get(major, minor);
  if (bd == NULL)
    return -ENODEV;

  debug(VFS, "mounting %s%zd\n", bd->driver->file_prefix, bd->minor);
  fs_t* fs = NULL;
  mutex_lock(&fs_list_lock);
  for (size_t i = 0; i < list_size(fs_list); i++)
  {
    fs_t* fs_ = list_get(fs_list, i);
    if (fs_->probe(bd) == SUCCESS)
    {
      debug(VFS, "successfully probed %s\n", fs_->name);
      fs = fs_;
      break;
    }
  }
  mutex_unlock(&fs_list_lock);

  if (fs == NULL)
    return -ENOTSUP;

  return fs->mount(bd, mountpoint);
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
