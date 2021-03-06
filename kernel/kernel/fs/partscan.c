#include <util/types.h>
#include <fs/blockdev.h>
#include <mm/memory.h>
#include <debug.h>
#include <util/string.h>

typedef struct
{
  bd_t* blkdev;
  size_t blk_offset;
  size_t blk_size;
  uint8_t type;
  char prefix[32];
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

ssize_t ps_read(void* drv_struct, size_t minor,
                char* buffer, size_t count, uint64_t lba)
{
  (void)minor;
  part_t* part = ((part_t*)drv_struct);
  return part->blkdev->driver->bd_ops.readblk(part->blkdev->data,
           part->blkdev->minor, buffer, count, lba + part->blk_offset);
}

ssize_t ps_write(void* drv_struct, size_t minor,
                char* buffer, size_t count, uint64_t lba)
{
  (void)minor;
  part_t* part = ((part_t*)drv_struct);
  return part->blkdev->driver->bd_ops.readblk(part->blkdev->data,
           part->blkdev->minor, buffer, count, lba + part->blk_offset);
}

const char* ps_get_prefix(void* drv_struct)
{
  return ((part_t*)drv_struct)->prefix;
}

static bd_driver_t partscan_drv = {
  .name = "partscan",
  .prefix = "p",
  .bd_ops = {
    .readblk = ps_read,
    .writeblk = ps_write,
    .get_prefix = ps_get_prefix
  }
};

static size_t part_major;

void partscan_init()
{
  part_major = bd_register_driver(&partscan_drv);
}

void partscan(bd_t* blkdev)
{
  if (blkdev->driver->major == part_major)
    return;
  debug(BLKDEV, "performing a partition scan on %s\n", blkdev->name);

  mbr_t mbr;
  if (blkdev->driver->bd_ops.readblk(blkdev->data, blkdev->minor,
                                 (char*)&mbr, 1, 0) < 1)
  {
    debug(BLKDEV, "cannot read from device\n");
    return;
  }

  if (mbr.magic[0] != 0x55 || mbr.magic[1] != 0xAA)
  {
    debug(BLKDEV, "disk has no MBR partition table\n");
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
    genpart->blkdev = blkdev;
    genpart->blk_offset = part->lba_start;
    genpart->blk_size = part->lba_size;
    genpart->type = part->type;
    sprintf(genpart->prefix, "%sp%zu", blkdev->name, i);

    bd_t* bd = kmalloc(sizeof(bd_t));
    bd->capacity = part->lba_size;
    bd->data = genpart;
    bd->driver = &partscan_drv;
    bd->minor = i;
    strcpy(bd->name, genpart->prefix);

    bd_register(bd);
  }
}
