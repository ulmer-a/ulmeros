#pragma once

#define O_WRONLY  0
#define O_TRUNC   0
#define O_CREAT   0
#define O_RDWR    0
#define O_RDONLY  0
#define O_APPEND  0
#define O_EXCL    0

#define S_IRUSR 0
#define S_IWUSR 0
#define S_IRGRP 0
#define S_IWGRP 0
#define S_IROTH 0
#define S_IWOTH 0

int open(const char *pathname, int flags, ...);
