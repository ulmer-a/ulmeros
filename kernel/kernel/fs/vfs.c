#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <fs/blockdev.h>
#include <util/string.h>
#include <arch/common.h>
#include <sched/task.h>
#include <mm/memory.h>
#include <debug.h>
#include <errno.h>
#include <time.h>

#define FMODE_DEFAULT {               \
  .u_r = 1, .u_w = 1, .u_x = 1,       \
  .g_r = 1, .g_w = 0, .g_x = 1,       \
  .o_r = 1, .o_w = 0, .o_x = 1,       \
  .sticky = 0,                        \
  .setgid = 0,                        \
  .setuid = 0                         \
}

dir_t* _vfs_root;

static list_t fs_list;
static mutex_t fs_list_lock;

int vfs_initialized = false;

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
    kpanic(false, "Cannot mount root file system\n");
    return;
  }

  fd_t* fd;
  int error;
  if ((error = bd_open(&fd, major, minor)) < 0)
  {
    debug(VFS, "error: cannot open device: %s\n", strerror(-error));
    kpanic(false, "Cannot mount root file system\n");
    return;
  }

  fs_t* fs;
  void* fsdata = probe_fs(fd, &fs);
  if (fsdata == NULL)
  {
    debug(VFS, "error: unknown file system\n");
    kpanic(false, "Cannot mount root file system\n");
    return;
  }

  dir_t* mp = fs->mount(fsdata);
  if (mp == NULL)
  {
    debug(VFS, "error: unknown file system\n");
    kpanic(false, "Cannot mount root file system\n");
    return;
  }

  VFS_ROOT = mp;
  mp->parent = mp;
  debug(VFS, "mounted root file system\n");

  debug(VFS, "mkdir /dev\n");
  fmode_t default_mode = FMODE_DEFAULT;
  vfs_mkdir(NULL, "/dev", default_mode);

  vfs_initialized = true;
  blockdev_mknodes();
}

static dir_t* get_working_dir(proc_t* proc)
{
  /* return a process' working directory. if the
   * working directory could not be determined,
   * the root directory will be returned. */
  if (proc && proc->working_dir)
    return proc->working_dir;
  return VFS_ROOT;
}

static file_t* falloc(proc_t* proc, ftype_t type, fmode_t mode)
{
  /* allocate a file structure with common defaults */
  file_t* file = kmalloc(sizeof(file_t));
  file->mode = mode;
  file->uid = proc ? proc->uid : 0;
  file->gid = proc ? proc->gid : 0;
  file->inode = 0;
  file->driver1 = NULL;
  file->driver2 = NULL;
  file->t_created =
      file->t_last_accessed =
      file->t_last_modified = time();
  file->type = type;
  file->length = 0;
  return file;
}


int vfs_mknod(proc_t *proc, const char *pathname,
              ftype_t type, fmode_t mode, size_t major, size_t minor)
{
  if (type != F_BLOCK && type != F_CHAR &&
      type != F_FIFO && type != F_SOCKET)
    return -EINVAL;

  /* check if the file exists. mknod() can only be used if
   * the file doesn't yet exist. */
  dir_t* working_dir = get_working_dir(proc);
  int status = ffind(working_dir, pathname, NULL, 0);
  if (status != -ENOENT)
  {
    if (status < 0)
      return status;
    return -EEXIST;
  }

  file_t* file = falloc(proc, type, mode);
  file->inode = 0;
  file->special.device.major = major;
  file->special.device.minor = minor;

  status = ffind(working_dir, pathname, &file, FFIND_CREATE);
  if (status < 0)
    kfree(file);
  return status;
}

int vfs_mkdir(proc_t* proc, const char* pathname, fmode_t mode)
{
  /* check if the file exists. mknod() can only be used if
   * the file doesn't yet exist. */
  dir_t* working_dir = get_working_dir(proc);
  int status = ffind(working_dir, pathname, NULL, 0);
  if (status != -ENOENT)
  {
    if (status < 0)
      return status;
    return -EEXIST;
  }

  dir_t* dir = kmalloc(sizeof(dir_t));
  dir->driver = NULL;
  list_init(&dir->files);
  dir->fstype = NULL;
  dir->mounted = NULL;

  file_t* file = falloc(proc, F_DIR, mode);
  file->special.directory = dir;
  file->inode = 0;
  dir->file = file;

  status = ffind(working_dir, pathname, &file, FFIND_CREATE);
  if (status < 0)
  {
    kfree(file);
    kfree(dir);
  }
  return status;
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

int vfs_open(const char *filename, int flags, int mode, fd_t** fd)
{
  dir_t* working_dir = VFS_ROOT;
  proc_t* process = NULL;
  if (current_task && current_task->process)
  {
    process = current_task->process;
    if (process->working_dir)
      working_dir = process->working_dir;
  }

  int error;
  file_t* target;
  if ((error = ffind(working_dir, filename, &target, 0)) < 0)
    return error;

  /* TODO: perform access control checks */

  if (target->type == F_REGULAR)
  {
    *fd = kmalloc(sizeof(fd_t));
    (*fd)->file = target;
    (*fd)->fpos = 0;
    assert(target->parent, "file_t has no parent!");
    (*fd)->f_ops = target->parent->fstype->f_ops;
    (*fd)->fs_data = target;
    return SUCCESS;
  }
  else if (target->type == F_BLOCK)
  {
    return bd_open(fd,
      target->special.device.major, target->special.device.minor);
  }
  else if (target->type == F_DIR)
  {
    return -EISDIR;
  }
  else
  {
    return -ENOSYS;
  }

  return -ENOTSUP;
}

void vfs_close(fd_t *fd)
{
  // TODO: implement
  kfree(fd);
}
