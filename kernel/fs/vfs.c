#include <types.h>
#include <errno.h>
#include <fs/vfs.h>
#include <list.h>
#include <mutex.h>
#include <arch.h>
#include <kstring.h>
#include <memory.h>

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

static void fs_fetch_dir(file_t* dirfile)
{
  assert(dirfile->type == F_DIR, "fs_fetch_dir(): file is not a director");
  dirfile->fs->fetch(dirfile);
}

static int namei_recursive(const char *pathname,
                           dir_t *working_dir,
                           file_t **node)
{
  char current_name[256];
  const char *rem = strccpy(current_name, pathname, '/');

  if (strlen(current_name) == 0)
    strcpy(current_name, ".");

  if (working_dir->mounted_fs != NULL)
    working_dir = working_dir->mounted_fs;
  if (list_size(working_dir->files) == 0)
    return -ENOENT;

  for (size_t i = 0; i < list_size(working_dir->files); i++)
  {
    direntry_t* entry = list_get(working_dir->files, i);

    if (strcmp(entry->name, current_name) == 0)
    {
      if (*rem == 0)
      {
        if (*(rem - 1) == '/' && entry->file->type != F_DIR)
          return -ENOTDIR;

        *node = entry->file;
        return SUCCESS;
      }

      if (entry->file->type != F_DIR)
        return -ENOENT;

      if (entry->file->dir == NULL)
        fs_fetch_dir(entry->file);
      return namei_recursive(rem, entry->file->dir, node);
    }
  }

  return -ENOENT;
}

int namei(const char *pathname, file_t **node)
{
  // TODO: replace with the actual working dir of the process
  dir_t *working_dir = &vfs_root_node;

  if (pathname[0] == '/')
  {
    // absolute path
    pathname += 1;
    working_dir = &vfs_root_node;

    if (pathname[1] == 0)
    {
      *node = working_dir->file;
      return SUCCESS;
    }
  }

  return namei_recursive(pathname, working_dir, node);
}

int vfs_open(const char* pathname, int flags, fd_t** fd)
{
  int error;
  file_t* direntry;
  if ((error = namei(pathname, &direntry)) < 0)
    return error;

  fd_t* new_fd = kmalloc(sizeof(fd_t));
  new_fd->seek_offset = 0;
  new_fd->refs = 1;
  new_fd->file = direntry;
  *fd = new_fd;
  return SUCCESS;
}

int vfs_close(fd_t* fd)
{
  assert(false, "not implemented");
  return -ENOSYS;
}

ssize_t vfs_read(fd_t* fd, char* buf, size_t len)
{
  file_t* file = fd->file;
  len = file->fs->read(file, buf, len, fd->seek_offset);
  if (len > 0)
    fd->seek_offset += len;
  return len;
}

ssize_t vfs_write(fd_t* fd, char* buf, size_t len)
{
  assert(false, "not implemented");
  return -ENOSYS;
}

int vfs_seek(fd_t* fd, ssize_t seek, int type)
{
  switch (type)
  {
  case SEEK_SET:
    if (seek < 0)
      return -EINVAL;
    fd->seek_offset = seek;
    break;
  case SEEK_END:
    fd->seek_offset = fd->file->length + seek;
    break;
  case SEEK_CUR:
    atomic_add(&fd->seek_offset, seek);
    break;
  default:
    return -EINVAL;
  }
  return SUCCESS;
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
