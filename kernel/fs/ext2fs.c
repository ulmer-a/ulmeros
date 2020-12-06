/*
 * ext2 filesystem driver
 * Written by Alexander Ulmer <ulmer@student.tugraz.at>
 * Copyright (C) 2017-2020
 *
 * Structs are taken from the linux kernel
 */

#include <errno.h>
#include <mutex.h>
#include <kstring.h>
#include <fs/vfs.h>
#include <fs/blockdev.h>
#include <memory.h>

#define LBA_SIZE  512 // Disk block size
#define SB_LBA    2   // 2 * 512
#define SB_SIZE   2   // 2 * 512

#define EXT2_NDIR_BLOCKS  12
#define EXT2_IND_BLOCK    EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK   (EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK   (EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS     (EXT2_TIND_BLOCK + 1)

typedef struct
{
  // base fields
  uint32_t total_inodes;
  uint32_t total_blocks;
  uint32_t root_blocks;
  uint32_t unalloc_blocks;
  uint32_t unalloc_inodes;
  uint32_t superblock;
  uint32_t block_size_log;
  uint32_t fragment_size_log;
  uint32_t blocks_per_group;
  uint32_t fragments_per_group;
  uint32_t inodes_per_group;
  uint32_t last_mount;
  uint32_t last_write;
  uint16_t mounts_since_check;
  uint16_t mounts_remaining;
  uint16_t signature;
  uint16_t fs_state;
  uint16_t error_action;
  uint16_t minor_version;
  uint32_t last_check;
  uint32_t check_interval;
  uint32_t creator_os;
  uint32_t major_version;
  uint16_t reserved_user;
  uint16_t reserved_group;

  // extended fields
  uint32_t first_inode_non_reserved;
  uint16_t inode_size;
  uint16_t backup_blockgroup;
  uint32_t optional_features;
  uint32_t required_features;
  uint32_t readonly_features;
  char fs_id[16];
  char vol_name[16];
  char last_mount_path[64];
  uint32_t compression_alg;
  uint8_t prealloc_file_blocks;
  uint8_t prealloc_dir_blocks;
  uint16_t unused;
  char journal_id[16];
  uint32_t journal_inode;
  uint32_t journal_device;
  uint32_t orphan_head;
} __attribute__((aligned(1024))) ext2_superblock_t;

typedef struct
{
    uint32_t bg_block_bitmap;       // Blocks bitmap block
    uint32_t bg_inode_bitmap;       // Inodes bitmap block
    uint32_t bg_inode_table;        // Inodes table block
    uint16_t bg_free_blocks_count;  // Free blocks count
    uint16_t bg_free_inodes_count;  // Free inodes count
    uint16_t bg_used_dirs_count;    // Directories count
} __attribute__((aligned(32))) ext2_gd_t;

typedef struct
{
  uint16_t  i_mode;           // File mode
  uint16_t  i_uid;            // Low 16 bits of Owner Uid
  uint32_t  i_size;           // Size in bytes
  uint32_t  i_atime;          // Access time
  uint32_t  i_ctime;          // Creation time
  uint32_t  i_mtime;          // Modification time
  uint32_t  i_dtime;          // Deletion Time
  uint16_t  i_gid;            // Low 16 bits of Group Id
  uint16_t  i_links_count;    // Links count
  uint32_t  i_blocks;         // Blocks count
  uint32_t  i_flags;          // File flags

  union {
    struct {
      uint32_t  l_i_reserved1;
    } linux1;
    struct {
      uint32_t  h_i_translator;
    } hurd1;
    struct {
      uint32_t  m_i_reserved1;
    } masix1;
  } osd1;             // OS dependent 1

  uint32_t  i_block[EXT2_N_BLOCKS]; // Pointers to blocks
  uint32_t  i_generation;           // File version (for NFS)
  uint32_t  i_file_acl;             // File ACL
  uint32_t  i_dir_acl;              // Directory ACL
  uint32_t  i_faddr;                // Fragment address

  union {
    struct {
      uint8_t    l_i_frag;   // Fragment number
      uint8_t    l_i_fsize;  // Fragment size
      uint16_t   i_pad1;
      uint16_t  l_i_uid_high;   // these 2 fields
      uint16_t  l_i_gid_high;   // were reserved2[0]
      uint32_t   l_i_reserved2;
    } linux2;
    struct {
      uint8_t    h_i_frag;   // Fragment number
      uint8_t    h_i_fsize;  // Fragment size
      uint16_t  h_i_mode_high;
      uint16_t  h_i_uid_high;
      uint16_t  h_i_gid_high;
      uint32_t  h_i_author;
    } hurd2;
    struct {
      uint8_t    m_i_frag;   // Fragment number
      uint8_t    m_i_fsize;  // Fragment size
      uint16_t   m_pad1;
      uint32_t   m_i_reserved2[2];
    } masix2;
  } osd2;             // OS dependent 2
} __attribute__((packed)) ext2_inode_t;

typedef struct
{
  uint32_t  inode;
  uint16_t  size;
  uint8_t   name_length;
  uint8_t   type; // feature dependent
} __attribute__((packed)) ext2_dentry_base_t;

typedef struct
{
  ext2_superblock_t sb;
  bd_t* disk;
  dir_t* root;
  size_t block_size;
  size_t gd_count;
  ext2_gd_t* group_descriptors;
  mutex_t fs_lock;
} ext2fs_t;

static ssize_t ext2_get_block(bd_t* disk, char* buffer, size_t lba, size_t count)
{
  ssize_t ret = disk->driver->
      fops.read(disk->minor, buffer, count, lba);
  if (ret < 0)
    return ret;
  return ret;
}

static int ext2_probe(bd_t* disk)
{
  ssize_t error;
  ext2_superblock_t sb;

  debug(EXT2FS, "probe: fetching supberblock from disk\n");
  if ((error = ext2_get_block(disk, (char*)&sb,
    SB_SIZE, SB_LBA)) < 0)
  {
    return error;
  }
  if (error < SB_SIZE)
  {
    return -EIO;
  }

  // if the signature matches, we're in
  uint16_t signature = sb.signature;
  if (signature == 0xef53)
  {
    return SUCCESS;
  }

  return -ENOTSUP;
}

static file_t* ext2_fetch_file(ext2fs_t* fs, size_t inode_no, int* error)
{
  if (inode_no > fs->sb.total_inodes)
  {
    if (error) *error = ENOENT;
    return NULL;
  }

  debug(EXT2FS, "fetching inode %zd\n", inode_no);
  mutex_lock(&fs->fs_lock);
  const size_t group_no = (inode_no - 1) / fs->sb.inodes_per_group;
  const ext2_gd_t* group = fs->group_descriptors + group_no;
  const size_t inode_size = (fs->sb.major_version < 1) ?
        128 : fs->sb.inode_size;
  const size_t inodes_per_sector = LBA_SIZE / inode_size;
  const size_t lba_offset = ((inode_no - 1) %
    fs->sb.inodes_per_group) / inodes_per_sector;
  const size_t itable_lba = lba_offset +
      (group->bg_inode_table * fs->block_size / LBA_SIZE);

  int err;
  char buffer[LBA_SIZE];
  if ((err = ext2_get_block(fs->disk, buffer, itable_lba, 1)) < 0)
  {
    mutex_unlock(&fs->fs_lock);
    if (error) *error = -err;
    return NULL;
  }
  mutex_unlock(&fs->fs_lock);

  // copy the inode into the heap
  // TODO: cache the inode, keep it in memory
  const size_t buffer_index = ((inode_no - 1) % inodes_per_sector);
  const ext2_inode_t* inode = (ext2_inode_t*)buffer + buffer_index;
  ext2_inode_t* drv_inode = kmalloc(sizeof(ext2_inode_t));
  *drv_inode = *inode;

  // allocate file structure and store info
  file_t* file = kmalloc(sizeof(file_t));
  file->type = inode->i_mode & 0xf000;
  uint16_t mode_only = inode->i_mode & 0x0fff;
  file->mode = *((fmode_t*)&mode_only);
  file->uid  = inode->i_uid;
  file->gid  = inode->i_gid;
  file->t_last_accessed = inode->i_atime;
  file->t_created       = inode->i_ctime;
  file->t_last_modified = inode->i_mtime;
  file->inode = inode_no;
  file->blocks = inode->i_blocks;
  file->length = inode->i_size;
  file->driver1 = fs;
  file->driver2 = drv_inode;
  mutex_init(&file->lock);
  return file;
}

static size_t ext2_get_file_block(ext2_inode_t* inode, size_t index)
{
  if (index < 12)
    return inode->i_block[index];
  assert(false, "ext2fs: indirect pointers are unimplemented");
  return 0;
}

static ssize_t ext2_read(file_t* file, char* buffer, size_t len, size_t offset)
{
  mutex_lock(&file->lock);

  // gather some information
  ext2fs_t* fs = file->driver1;
  ext2_inode_t* inode = file->driver2;

  // truncate bytes to read if necessary
  if (offset + len > file->length)
    len = file->length - offset;

  debug(EXT2FS, "read: inode %zd, off=0x%zx, size=%zd%s\n",
        file->inode, offset,
        (len < 1024) ? len : len >> 10,
        (len < 1024) ? "B" : "K");

  int error = SUCCESS;
  size_t bytes_read = 0;
  char* block_buffer = kmalloc(fs->block_size);
  while (bytes_read < len)
  {
    const size_t current_block = (offset + bytes_read) / fs->block_size;
    const size_t block_no = ext2_get_file_block(inode, current_block);
    const size_t block_lba = block_no * fs->block_size / LBA_SIZE;
    const size_t block_lba_size = fs->block_size / LBA_SIZE;

    int err;
    if ((err = ext2_get_block(fs->disk, block_buffer, block_lba, block_lba_size)) < 0)
    {
      error = -err;
      break;
    }

    ssize_t bytes_to_copy = fs->block_size;
    if (len < fs->block_size)
        bytes_to_copy = len % fs->block_size;
    memcpy(buffer, block_buffer, bytes_to_copy);
    bytes_read += bytes_to_copy;
  }
  kfree(block_buffer);
  mutex_unlock(&file->lock);

  if (error != SUCCESS)
    return error;
  return bytes_read;
}

static void ext2_load_files(dir_t* dir)
{
  // TODO: assert that lock is already acquired!

  file_t *dfile = dir->file;
  assert(dfile, "ext2_load_files(): dfile is NULL");
  ext2fs_t* fs = dfile->driver1;

  char* buffer = kmalloc(dfile->length);
  ext2_read(dfile, buffer, dfile->length, 0);

  list_clear(dir->files);
  size_t index = 0;
  while (index < dfile->length)
  {
    ext2_dentry_base_t* direntry =
        (ext2_dentry_base_t*)(buffer + index);
    index += direntry->size;

    if (direntry->inode != 0)
    {
      direntry_t* dentry = kmalloc(sizeof(direntry_t));

      // copy the name and fetch the inode
      char* name = (char*)(direntry + 1);
      strncpy(dentry->name, name, direntry->name_length);
      dentry->name[direntry->name_length] = 0;
      dentry->file = ext2_fetch_file(fs, direntry->inode, NULL);

      list_add(dir->files, dentry);
    }
  }

  kfree(buffer);
}

static dir_t* ext2_fetch_dir(ext2fs_t* fs, size_t inode, int* error)
{
  file_t* dfile = ext2_fetch_file(fs, inode, error);
  if (dfile == NULL)
    return NULL;

  if (dfile->type != F_DIR)
  {
    if (error) *error = ENOTDIR;
    return NULL;
  }

  mutex_lock(&dfile->lock);
  dir_t* dir = dfile->dir;
  if (dir == NULL)
  {
    dir = kmalloc(sizeof(dir_t));
    dir->file = dfile;
    dir->mounted_fs = NULL;
    dir->files = list_init();
    dfile->dir = dir;

    ext2_load_files(dir);
  }
  mutex_unlock(&dfile->lock);
  return dir;
}

static void ext2_delfs(ext2fs_t* fsdata)
{
  // flush
  // free all cached inodes

  kfree(fsdata->group_descriptors);
  kfree(fsdata);
}

static int ext2_mount(bd_t* disk, dir_t* mp)
{
  ssize_t error;
  ext2_superblock_t sb;

  debug(EXT2FS, "mount: fetching supberblock from disk\n");
  // fetch the superblock from disk
  if ((error = ext2_get_block(disk, (char*)&sb,
    SB_SIZE, SB_LBA)) < 0)
      return error;
  if (error < SB_SIZE)
    return -EIO;

  // if the signature matches, we're in
  uint16_t signature = sb.signature;
  if (signature != 0xef53)
      return -ENOTSUP;

  debug(EXT2FS, "mount: generating fs structs\n");
  ext2fs_t* fsdata = kmalloc(sizeof(ext2fs_t));
  fsdata->disk = disk;
  fsdata->sb = sb;
  fsdata->block_size = 0x400 << fsdata->sb.block_size_log;
  fsdata->gd_count = fsdata->sb.total_blocks / fsdata->sb.blocks_per_group;
  if (fsdata->sb.total_blocks % fsdata->sb.blocks_per_group)
    fsdata->gd_count += 1;

  debug(EXT2FS, "mount: fetching group descriptor table\n");
  size_t gdt_lba = (fsdata->block_size == 0x400) ? 2 : 1;
  gdt_lba *= fsdata->block_size / LBA_SIZE;
  size_t gdt_size = fsdata->gd_count * sizeof(ext2_gd_t);
  size_t gdt_lba_size = gdt_size / LBA_SIZE;
  if (gdt_size % LBA_SIZE != 0)
      gdt_lba_size += 1;

  fsdata->group_descriptors = kmalloc(gdt_lba_size * LBA_SIZE);
  if ((error = ext2_get_block(disk, (char*)fsdata->group_descriptors,
                         gdt_lba, gdt_lba_size)) < 0)
  {
    ext2_delfs(fsdata);
    return -EIO;
  }

  debug(EXT2FS, "mount: fetching root inode\n");
  fsdata->root = ext2_fetch_dir(fsdata, 2, NULL);
  assert(fsdata->root != NULL, "ext2_mount(): root inode is NULL");
  mp->mounted_fs = fsdata->root;
  return SUCCESS;
}

static const fs_t ext2fs = {
  .name = "ext2fs",
  .probe = ext2_probe,
  .mount = ext2_mount,
  .read = ext2_read,
  .mbr_id = 0x83
};

void ext2fs_load()
{
  debug(EXT2FS, "loading ext2fs filesystem driver\n");

  assert(sizeof(ext2_inode_t) == 128, "ext2 inode_t struct size");
  assert(sizeof(ext2_gd_t) == 32, "ext2 gd_t struct size");
  assert(sizeof(ext2_superblock_t) == 1024, "ext2 sb_t struct size");

  register_fs(&ext2fs);
}
