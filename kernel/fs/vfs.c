#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <fs/blockdev.h>
#include <util/string.h>
#include <arch/common.h>
#include <sched/task.h>
#include <mm/memory.h>
#include <debug.h>
#include <errno.h>

dir_t* _vfs_root;

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

  dir_t* mp = fs->mount(fsdata);
  if (mp == NULL)
  {
    debug(VFS, "error: unknown file system\n");
    assert(false, "Cannot mount root file system\n");
    return;
  }

  VFS_ROOT = mp;
  mp->parent = mp;
  debug(VFS, "mounted root file system\n");
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

static void fs_fetch(dir_t* parent, direntry_t* entry)
{
  if (entry->file != NULL)
    return;
  assert(parent->fstype, "no filesystem info in directory");
  parent->fstype->fetch(parent, entry);
  entry->file->parent = parent;
}

static int namei_recursive(const char *pathname,
                           dir_t *working_dir,
                           file_t **node)
{
  char current_name[256];
  const char *rem = strccpy(current_name, pathname, '/');

  if (strlen(current_name) == 0)
    strcpy(current_name, ".");

  if (working_dir->mounted != NULL)
    working_dir = working_dir->mounted;
  if (list_size(&working_dir->files) == 0)
    return -ENOENT;

  for (list_item_t* it = list_it_front(&working_dir->files);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    direntry_t* entry = list_it_get(it);

    if (strcmp(entry->name, current_name) == 0)
    {
      if (entry->file == NULL)
        fs_fetch(working_dir, entry);

      if (*rem == 0)
      {
        if (*(rem - 1) == '/' && entry->file->type != F_DIR)
          return -ENOTDIR;

        *node = entry->file;
        return SUCCESS;
      }

      if (entry->file->type != F_DIR)
        return -ENOENT;
      return namei_recursive(rem, entry->file->special.directory, node);
    }
  }

  return -ENOENT;
}

int namei(dir_t* working_dir, const char *pathname, file_t **node)
{
  if (pathname[0] == '/')
  {
    // absolute path
    pathname += 1;
    working_dir = VFS_ROOT;

    if (pathname[1] == 0)
    {
      *node = working_dir->file;
      return SUCCESS;
    }
  }

  return namei_recursive(pathname, working_dir, node);
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
  if ((error = namei(working_dir, filename, &target)) < 0)
    return error;

  /* TODO: perform access control checks */

  if (target->type == F_REGULAR)
  {
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
