#include <fs/vfs.h>
#include <sched/proc.h>
#include <sched/task.h>
#include <arch/common.h>
#include <syscalls.h>
#include <debug.h>
#include <errno.h>

// TODO: buffer & range checks for userspace!

#define SYS_BUFFER_RANGE_CHECK(buf, size)           \
  if ((size_t)(buf) + (size_t)(size) > USER_BREAK)  \
    return -EINVAL;

ssize_t sys_read(int fd, char* buffer, size_t len)
{
  SYS_BUFFER_RANGE_CHECK(buffer, len);

  assert(current_task && current_task->process,
         "sys_open() called from kernel mode. use vfs_read()");

  fd_t* fdp = proc_get_fd(current_task->process, fd);
  if (fdp == NULL)
    return -EBADF;

  if (!fdp->f_ops.read)
  {
    mutex_unlock(&fdp->fdmod);
    return -ENOTSUP;
  }

  ssize_t ret = fdp->f_ops.read(fdp->fs_data, buffer, len, fdp->fpos);
  mutex_unlock(&fdp->fdmod);
  return ret;
}

ssize_t sys_write(int fd, char* buffer, size_t len)
{
  SYS_BUFFER_RANGE_CHECK(buffer, len);

  assert(current_task && current_task->process,
         "sys_open() called from kernel mode. use vfs_write()");

  fd_t* fdp = proc_get_fd(current_task->process, fd);
  if (fdp == NULL)
    return -EBADF;

  if (!fdp->f_ops.write)
  {
    mutex_unlock(&fdp->fdmod);
    return -ENOTSUP;
  }

  ssize_t ret = fdp->f_ops.write(fdp->fs_data, buffer, len, fdp->fpos);
  mutex_unlock(&fdp->fdmod);
  return ret;
}

int sys_open(char* path, int flags, int mode)
{
  assert(current_task && current_task->process,
         "sys_open() called from kernel mode. use vfs_open()");

  /* open a file. this will create a new file descriptor
   * instance and increase reference counts accordingly. */
  fd_t* fd;
  int error;
  if ((error = vfs_open(path, flags, mode, &fd)) < 0)
    return error;

  /* allocate a new file descriptor number for the process. */
  return proc_new_fd(current_task->process, fd);
}

int sys_close(int fd)
{
  return -ENOSYS;
}
