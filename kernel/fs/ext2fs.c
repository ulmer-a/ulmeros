#include <util/types.h>
#include <util/mutex.h>
#include <util/list.h>
#include <util/string.h>
#include <mm/memory.h>
#include <fs/vfs.h>
#include <debug.h>
#include <errno.h>

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
  fd_t* fd;
  dir_t* root;
  size_t block_size;
  size_t gd_count;
  ext2_gd_t* group_descriptors;
  mutex_t lock;
} ext2fs_t;

static int ext2_get_block(ext2fs_t* fs,
    void* buffer, size_t lba, size_t count)
{
  ssize_t error;
  mutex_lock(&fs->lock);
  vfs_seek(fs->fd, lba * BLOCK_SIZE, SEEK_SET);
  error = vfs_read(fs->fd, buffer, count * BLOCK_SIZE);
  mutex_unlock(&fs->lock);
  if (error < count * BLOCK_SIZE)
  {
    debug(EXT2FS, "ext2 I/O error: %s\n", strerror(-error));
    return false;
  }
  return true;
}

static void* ext2_probe(fd_t* fd)
{
  ssize_t error;
  ext2_superblock_t sb;
  vfs_seek(fd, SB_LBA * BLOCK_SIZE, SEEK_SET);
  if ((error = vfs_read(fd, &sb, SB_SIZE * BLOCK_SIZE)) < 0)
  {
    debug(EXT2FS, "error: cannot read from disk\n", strerror(-error));
    return NULL;
  }

  // if the signature matches, we're in
  uint16_t signature = sb.signature;
  if (signature != 0xef53)
    return NULL;

  ext2fs_t* fs = kmalloc(sizeof(ext2fs_t));
  mutex_init(&fs->lock);
  fs->root = NULL;
  fs->fd = vfs_dup(fd);
  fs->sb = sb;
  fs->block_size = 0x400 << fs->sb.block_size_log;
  fs->gd_count = fs->sb.total_blocks / fs->sb.blocks_per_group;
  if (fs->sb.total_blocks % fs->sb.blocks_per_group)
    fs->gd_count += 1;

  size_t gdt_lba = (fs->block_size == 0x400) ? 2 : 1;
  gdt_lba *= fs->block_size / LBA_SIZE;
  size_t gdt_size = fs->gd_count * sizeof(ext2_gd_t);
  size_t gdt_lba_size = gdt_size / LBA_SIZE;
  if (gdt_size % LBA_SIZE != 0)
      gdt_lba_size += 1;

  fs->group_descriptors = kmalloc(gdt_lba_size * LBA_SIZE);
  if (!ext2_get_block(fs, fs->group_descriptors, gdt_lba, gdt_lba_size))
  {
    kfree(fs->group_descriptors);
    kfree(fs);
    return NULL;
  }

  return fs;
}

static fs_t ext2fs = {
  .name = "ext2fs",
  .probe = ext2_probe,
  .mbr_id = 0x83,
};

void ext2fs_init()
{
  debug(EXT2FS, "initializing ext2 filesystem driver\n");

  fs_register(&ext2fs);
}
