#pragma once

#include <stdlib.h>

ssize_t read(int fd, char* buffer, size_t len);

ssize_t write(int fd, char* buffer, size_t len);