#pragma once

#include <util/types.h>

size_t devfs_add(const char* name, size_t major, size_t minor);
void devfs_unlink(size_t inode);
