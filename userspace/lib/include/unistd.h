#pragma once

#include <sys/types.h>

ssize_t read(int fd, char* buffer, size_t len);
ssize_t write(int fd, char* buffer, size_t len);

int close(int fd);
