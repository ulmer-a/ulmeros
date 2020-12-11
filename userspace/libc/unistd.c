#include <unistd.h>
#include <errno.h>

ssize_t read(int fd, char* buffer, size_t len)
{
  return -ENOSYS;
}

ssize_t write(int fd, char* buffer, size_t len)
{
  return -ENOSYS;
}