#pragma once

#include <types.h>
#include <fs/blockdev.h>
#include <list.h>
#include <mutex.h>

#define BLOCK_SIZE 512

struct _file;
typedef struct _file file_t;

struct _dir;
typedef struct _dir dir_t;

typedef struct
{
  const char *name;
  uint8_t mbr_id;

  int (*probe)(bd_t* disk);
  int (*mount)(bd_t* disk, dir_t* mp);
  int (*umount)();
  ssize_t (*read)(file_t* file, char* buffer,
      size_t len, size_t offset);
  ssize_t (*write)(file_t* file, char* buffer,
      size_t len, size_t offset);
} fs_t;

typedef enum
{
  F_FIFO      = 0x1000,
  F_CHAR      = 0x2000,
  F_DIR       = 0x4000,
  F_BLOCK     = 0x6000,
  F_REGULAR   = 0x8000,
  F_SYMLINK   = 0xa000,
  F_SOCKET    = 0xc000,
  F_UNKNOWN   = 0x0000
} ftype_t;

typedef struct
{
  uint16_t o_x      : 1;
  uint16_t o_w      : 1;
  uint16_t o_r      : 1;
  uint16_t g_x      : 1;
  uint16_t g_w      : 1;
  uint16_t g_r      : 1;
  uint16_t u_x      : 1;
  uint16_t u_w      : 1;
  uint16_t u_r      : 1;
  uint16_t sticky   : 1;
  uint16_t setgid   : 1;
  uint16_t setuid   : 1;
  uint16_t _unused  : 4;
} fmode_t;

typedef struct _dir
{
  file_t* file;
  dir_t* mounted_fs;
  list_t* files;
} dir_t;

typedef struct _file
{
  ftype_t type;   // unix file type
  fmode_t mode;   // mode, permissions, ...

  size_t uid;
  size_t gid;

  size_t t_last_modified;
  size_t t_last_accessed;
  size_t t_created;

  size_t length;  // file size
  size_t blocks;  // size in blocks

  size_t inode;   // inode number

  dir_t* parent;  // parent directory
  dir_t* dir;     // valid if type=DIR

  void *driver1;
  void *driver2;

  mutex_t lock;
} file_t;

typedef struct _dentry
{
  char name[256];
  file_t *file;
} direntry_t;



void vfs_init();

void register_fs(const fs_t* fs);
