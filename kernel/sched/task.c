#include <syscalls.h>
#include <task.h>
#include <errno.h>
#include <arch/context.h>

arch_context_t* saved_context_;

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



void set_context(arch_context_t* context)
{
  saved_context_ = context;
}

arch_context_t* get_context()
{
  return saved_context_;
}
