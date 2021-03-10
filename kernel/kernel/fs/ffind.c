#include <fs/vfs.h>
#include <sched/mutex.h>
#include <errno.h>
#include <debug.h>
#include <util/string.h>

static mutex_t fs_tree_lock = MUTEX_INITIALIZER;


static void fs_fetch(dir_t* parent, direntry_t* entry)
{
  if (entry->file != NULL)
    return;
  assert(parent->fstype, "no filesystem info in directory");
  parent->fstype->fetch(parent, entry);
  entry->file->parent = parent;
}

static int ffind_noent(dir_t* parent, int flags, file_t* new_file)
{
  if ((flags & FFIND_CREATE) != 0)
    return -ENOENT;

  list_add(&parent->files, new_file);
  new_file->parent = parent;
  new_file->driver1 = parent->file->driver1;
  new_file->driver2 = NULL;

  if (new_file->type == F_DIR)
  {
    new_file->special.directory->parent = parent;
    new_file->special.directory->driver = new_file->driver1;
  }
  return SUCCESS;
}

static int ffind_success(int flags)
{
  if (flags & FFIND_CREATE)
    return -EEXIST;
  return SUCCESS;
}

static int ffind_recursive(const char* pathname, dir_t* working_dir,
                           file_t** node, int flags)
{
  if (strclen(pathname, '/') > 255)
    return -ENAMETOOLONG;
  char current_name[256];
  const char *rem = strccpy(current_name, pathname, '/');

  if (strlen(current_name) == 0)
    strcpy(current_name, ".");

  if (working_dir->mounted != NULL)
    working_dir = working_dir->mounted;

  if (list_size(&working_dir->files) == 0)
    return ffind_noent(working_dir, flags, *node);

  for (list_item_t* it = list_it_front(&working_dir->files);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    direntry_t* entry = list_it_get(it);

    if (strcmp(entry->name, current_name) == 0)
    {
      if (entry->file == NULL)
        fs_fetch(working_dir, entry);

      if (*rem == 0)
      {
        if (*(rem - 1) == '/' && entry->file->type != F_DIR)
          return -ENOTDIR;

        if (*node)
          *node = entry->file;
        return ffind_success(flags);
      }

      if (entry->file->type != F_DIR)
        return ffind_noent(working_dir, flags, *node);
      return ffind_recursive(rem, entry->file->special.directory, node, flags);
    }
  }

  return ffind_noent(working_dir, flags, *node);
}

int ffind(dir_t* working_dir, const char* pathname, file_t** node, int flags)
{
  if (pathname[0] == '/')
  {
    pathname += 1;
    working_dir = VFS_ROOT;

    if (pathname[1] == 0)
    {
      if (flags & FFIND_CREATE)
        return -EEXIST;
      if (*node)
        *node = working_dir->file;
      return SUCCESS;
    }
  }

  mutex_lock(&fs_tree_lock);
  int ret = ffind_recursive(pathname, working_dir, node, flags);
  mutex_unlock(&fs_tree_lock);
  return ret;
}
