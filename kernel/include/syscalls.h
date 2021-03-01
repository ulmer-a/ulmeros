#pragma once

/* this file contains system call definitions. these
 * functions are available to userspace to allow the
 * invocation of kernel services. */

#include <util/types.h>

/* processes and flow control */
void      sys_exit();
ssize_t   sys_fork();
int       sys_exec(char *path, char** argv);
int       sys_wait(size_t pid, int* exit_code);
size_t    sys_getpid();

/* filesystem access */
ssize_t   sys_read(int fd, char* buffer, size_t len);
ssize_t   sys_write(int fd, char* buffer, size_t len);
int       sys_open(char* path, int flags, int mode);
int       sys_close(int fd);

/* interprocess communication */
int       sys_pipe(int* read_end, int* write_end);
