#include <fs/blockdev.h>

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

void partscan(bd_t* disk)
{
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
  }
}

