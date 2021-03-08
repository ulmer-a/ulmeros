#pragma once

#include <util/types.h>
#include <util/list.h>
#include <sched/mutex.h>

#define BLOCK_SIZE 512

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

#define MODE_O_X      BIT(0)
#define MODE_O_W      BIT(1)
#define MODE_O_R      BIT(2)
#define MODE_G_X      BIT(3)
#define MODE_G_W      BIT(4)
#define MODE_G_R      BIT(5)
#define MODE_U_X      BIT(6)
#define MODE_U_W      BIT(7)
#define MODE_U_R      BIT(8)
#define MODE_STICKY   BIT(9);
#define MODE_SETGID   BIT(10);
#define MODE_SETUID   BIT(11);

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

struct _fs_struct;
typedef struct _fs_struct fs_t;

struct _file_struct;
typedef struct _file_struct file_t;

struct _dir_struct;
typedef struct _dir_struct dir_t;

struct _fd_struct;
typedef struct _fd_struct fd_t;

typedef struct _dentry
{
  char name[256];
  size_t inode;
  file_t *file;
} direntry_t;

typedef struct
{
  ssize_t (*read)(void* fsdata, void* buffer, size_t len, uint64_t off);
  ssize_t (*write)(void* fsdata, void* buffer, size_t len, uint64_t off);
} f_ops_t;

struct _fd_struct
{
  file_t* file;   // pointer to file object
  uint64_t fpos;  // seek position

  f_ops_t f_ops;    // file operations
  void* fs_data;  // driver/filesystem data (bd_t, inode)

  mutex_t fdmod;  // modification lock
};

struct _dir_struct
{
  file_t* file;   // file object of the directory
  list_t files;  // list of direntries
  dir_t* parent;  // parent directory (null if mount point)
  dir_t* mounted; // root directory if dir is mountpoint
  fs_t* fstype;   // file system type structure
  void* driver;   // pointer to fs instance
};

typedef struct
{
  uint32_t major;
  uint32_t minor;
} sp_dev_t;

typedef union
{
  dir_t* directory;         // pointer to dir_t, if type = directory
  file_t* symlink;          // pointer to file_t, if type = symlink
  sp_dev_t device;          // major/minor pair, if type = char/blkdev
} sp_file_u;

struct _file_struct
{
  fmode_t mode;             // file mode
  ftype_t type;             // file type
  uint32_t uid;               // user id
  uint32_t gid;               // group id
  uint64_t t_last_modified; // last modification time
  uint64_t t_last_accessed; // last access time
  uint64_t t_created;       // creation time
  uint64_t length;          // file size
  size_t inode;
  void* driver1;
  void* driver2;
  dir_t* parent;
  sp_file_u special;        // special file features
};

struct _fs_struct
{
  const char* name;
  uint8_t mbr_id;
  void* (*probe)(fd_t* fd);
  dir_t* (*mount)(void* fs);
  void (*fetch)(dir_t* parent, direntry_t* direntry);
  f_ops_t f_ops;
};

extern dir_t* _vfs_root;
#define VFS_ROOT (_vfs_root)

void vfs_init(const char *rootfs);

void fs_register(fs_t *fs);

#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

#define O_RDONLY BIT(0)

int vfs_open(const char* filename, int flags, int mode, fd_t **fd);
void vfs_close(fd_t* fd);
ssize_t vfs_read(fd_t* fd, void* buffer, uint64_t length);
uint64_t vfs_seek(fd_t* fd, uint64_t offset, int whence);
fd_t* vfs_dup(fd_t* fd);
