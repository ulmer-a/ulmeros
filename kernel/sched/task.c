#include <syscalls.h>
#include <task.h>
#include <errno.h>

void sys_exit()
{

}

ssize_t sys_fork()
{
  return -ENOSYS;
}

int sys_exec(char *path, char** argv)
{
  return -ENOSYS;
}

int sys_wait(size_t pid, int* exit_code)
{
  return -ENOSYS;
}

size_t sys_getpid()
{
  return (size_t)-ENOSYS;
}
