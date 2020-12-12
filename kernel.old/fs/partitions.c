#include <fs/blockdev.h>
#include <memory.h>
#include <arch.h>
#include <list.h>
#include <mutex.h>
#include <errno.h>

typedef struct
{
  bd_t* disk;
  size_t lba_start;
  size_t lba_size;
  uint8_t type;
} part_t;

typedef struct
{
  uint8_t flags;
  uint8_t chs_start[3];
  uint8_t type;
  uint8_t chs_last[3];
  uint32_t lba_start;
  uint32_t lba_size;
} __attribute__((packed)) mbr_part_t;

typedef struct
{
  char code[440];
  uint32_t signature;
  uint16_t zero;
  mbr_part_t parts[4];
  unsigned char magic[2];
} __attribute((packed)) mbr_t;

static list_t* part_list;
static mutex_t part_list_lock = MUTEX_INITIALIZER;

ssize_t parts_read(size_t minor, char *buf,
                size_t count, uint64_t lba)
{
  mutex_lock(&part_list_lock);
  part_t* part = list_get(part_list, minor);
  mutex_unlock(&part_list_lock);

  // cut down count if too much is requested
  if (count > part->lba_size - lba)
    count = part->lba_size - lba;

  // add offset
  lba += part->lba_start;
  if (lba >= part->lba_start + part->lba_size)
    return -EIO;

  return part->disk->driver->fops.read(
        part->disk->minor, buf, count, lba);
}

ssize_t parts_write(size_t minor, char *buf,
                 size_t count, uint64_t lba)
{
  mutex_lock(&part_list_lock);
  part_t* part = list_get(part_list, minor);
  mutex_unlock(&part_list_lock);

  // cut down count if too much is requested
  if (count > part->lba_size - lba)
    count = part->lba_size - lba;

  // add offset
  lba += part->lba_start;
  if (lba >= part->lba_start + part->lba_size)
    return -EIO;

  return part->disk->driver->fops.write(
        part->disk->minor, buf, count, lba);
}

static bd_driver_t parts_driver = {
  .name = "partdev",
  .file_prefix = "disk",
  .fops = {
    .read = parts_read,
    .write = parts_write
  }
};

void parts_init()
{
  part_list = list_init();
  bd_register_driver(&parts_driver);
}

void partscan(bd_t* disk)
{
  if (disk->driver->major == parts_driver.major)
    return;

  debug(BLKDEV, "partscan on disk %s%zd\n",
        disk->driver->file_prefix, disk->minor);

  mbr_t mbr;
  ssize_t status = disk->driver->fops.read(
        disk->minor, (char*)&mbr, 1, 0);

  if (status < 1)
  {
    debug(BLKDEV, "%s%zd: cannot read from disk\n",
          disk->driver->file_prefix, disk->minor);
    return;
  }

  if (mbr.magic[0] != 0x55 || mbr.magic[1] != 0xaa)
  {
    debug(VFS, "partscan failed: disk has no MBR partition table\n");
    return;
  }

  for (size_t i = 0; i < 4; i++)
  {
    mbr_part_t* part = &(mbr.parts[i]);
    if (part->type == 0x00)
    {
      debug(BLKDEV, " #%zd: empty\n", i);
      continue;
    }

    debug(BLKDEV, " #%zd: start_lba=%zd, sectors=%zd, type=0x%x\n",
          i, part->lba_start, part->lba_size, part->type);

    part_t* genpart = kmalloc(sizeof(part_t));
    genpart->disk = disk;
    genpart->lba_start = part->lba_start;
    genpart->lba_size = part->lba_size;
    genpart->type = part->type;

    bd_t* bd = kmalloc(sizeof(bd_t));
    bd->capacity = part->lba_size;
    bd->driver = &parts_driver;

    mutex_lock(&part_list_lock);
    bd->minor = list_add(part_list, genpart);
    mutex_unlock(&part_list_lock);

    bd_register(bd);
  }
}

