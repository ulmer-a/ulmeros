#include <util/types.h>
#include <sched/mutex.h>
#include <util/string.h>
#include <util/list.h>
#include <mm/memory.h>
#include <arch/common.h>
#include <fs/vfs.h>
#include <debug.h>

typedef struct
{
  list_t children;
  char name[256];
  fmode_t mode;
  ftype_t type;
  void* content;
  uint64_t size;
} ramfs_file_t;

typedef struct
{
  size_t id;
  list_t files;
  mutex_t fs_lock;
} ramfs_instance_t;

static size_t ramfs_id_counter = 1;

void* ramfs_create()
{
  ramfs_instance_t* ramfs = kmalloc(sizeof(ramfs_instance_t));
  mutex_init(&ramfs->fs_lock);
  list_init(&ramfs->files);
  ramfs->id = atomic_add(&ramfs_id_counter, 1);
  debug(RAMFS, "creating new ramfs filesystem #%zu\n", ramfs->id);
  return ramfs;
}

static const fs_t ramfs_info = {
  .name = "ramfs",
};

const fs_t* ramfs_get_info()
{
  return &ramfs_info;
}
